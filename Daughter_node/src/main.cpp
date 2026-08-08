#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_bt.h>

// =====================================================
// BUTTON PINS
// =====================================================
#define INTRUSION_BUTTON_PIN 27
#define SOS_BUTTON_PIN 25

// =====================================================
// ESP-NOW CHANNEL (must match Mother Node + router channel)
// =====================================================
#define ESPNOW_CHANNEL 2

// =====================================================
// ALERT CODES (must match Mother Node)
// =====================================================
#define MSG_INTRUSION 1
#define MSG_SOS 2

// =====================================================
// MOTHER NODE MAC ADDRESS
// =====================================================
uint8_t receiverMAC[] = {
    0xB0, 0xCB, 0xD8,
    0xC6, 0xA5, 0xA8};

// =====================================================
// ESP-NOW MESSAGE (must match Mother Node)
// =====================================================
typedef struct struct_message
{
  int value;
} struct_message;

struct_message outgoingData;

esp_now_peer_info_t peerInfo;

// =====================================================
// BUTTON STATES
// =====================================================
bool lastIntrusionState = HIGH;
bool lastSOSState = HIGH;

// =====================================================
// SEND STATE (used for retry-on-fail)
// =====================================================
volatile bool lastSendFailed = false;

// =====================================================
// SEND CALLBACK
// =====================================================
void OnDataSent(
    const uint8_t *mac_addr,
    esp_now_send_status_t status)
{
  lastSendFailed = (status != ESP_NOW_SEND_SUCCESS);

  Serial.println();
  Serial.println("[ESPNOW] ===== SEND CALLBACK =====");
  Serial.printf(
      "[ESPNOW] Receiver: %02X:%02X:%02X:%02X:%02X:%02X\n",
      mac_addr[0],
      mac_addr[1],
      mac_addr[2],
      mac_addr[3],
      mac_addr[4],
      mac_addr[5]);

  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println(
        "[ESPNOW] STATUS: SUCCESS");
  }
  else
  {
    Serial.println(
        "[ESPNOW] STATUS: FAILED");
  }

  Serial.println(
      "[ESPNOW] =========================");
}

// =====================================================
// SEND MESSAGE (retries up to 3 times if no ACK)
// =====================================================
void sendMessage(
    int messageValue,
    const char *messageName)
{
  outgoingData.value = messageValue;

  Serial.println();
  Serial.println(
      "[SEND] ===============================");

  Serial.printf(
      "[SEND] Event: %s\n",
      messageName);

  Serial.printf(
      "[SEND] Value: %d\n",
      outgoingData.value);

  Serial.printf(
      "[SEND] Packet size: %d bytes\n",
      sizeof(outgoingData));

  for (int attempt = 1; attempt <= 3; attempt++)
  {
    lastSendFailed = false;

    esp_err_t result =
        esp_now_send(
            receiverMAC,
            (uint8_t *)&outgoingData,
            sizeof(outgoingData));

    Serial.printf(
        "[SEND] esp_now_send(): %s (0x%x)\n",
        (result == ESP_OK)
            ? "ESP_OK"
            : esp_err_to_name(result),
        result);

    if (result != ESP_OK)
    {
      Serial.println(
          "[SEND] ERROR: Failed to start transmission");
      break;
    }

    delay(100); // give the callback time to report the ACK verdict

    if (!lastSendFailed)
    {
      Serial.println(
          "[SEND] Delivery confirmed (ACK)");
      break;
    }

    if (attempt < 3)
    {
      Serial.printf(
          "[SEND] Attempt %d/3: no ACK, retrying...\n",
          attempt);
    }
    else
    {
      Serial.println(
          "[SEND] ERROR: Delivery failed after 3 attempts");
    }
  }

  Serial.println(
      "[SEND] ===============================");
}

// =====================================================
// SETUP
// =====================================================
void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println(
      "========================================");

  Serial.println(
      "       LAKSHMAN REKHA");

  Serial.println(
      "          DAUGHTER NODE");

  Serial.println(
      "========================================");

  // =================================================
  // BUTTONS
  // =================================================

  pinMode(
      INTRUSION_BUTTON_PIN,
      INPUT_PULLUP);

  pinMode(
      SOS_BUTTON_PIN,
      INPUT_PULLUP);

  Serial.printf(
      "[BUTTON] Intrusion button: GPIO %d\n",
      INTRUSION_BUTTON_PIN);

  Serial.printf(
      "[BUTTON] SOS button: GPIO %d\n",
      SOS_BUTTON_PIN);

  Serial.println(
      "[BUTTON] Using internal pull-up resistors");

  // =================================================
  // WIFI RADIO
  // =================================================

  WiFi.mode(WIFI_STA);

  // Disable Wi-Fi sleep for better ESP-NOW
  // responsiveness.

  WiFi.setSleep(false);

  // Free the radio from Bluetooth coexistence.

  btStop();

  Serial.println(
      "[WIFI] Mode: STA");

  Serial.println(
      "[WIFI] Sleep: DISABLED");

  Serial.println(
      "[WIFI] BT: OFF");

  Serial.print(
      "[WIFI] Daughter MAC: ");

  Serial.println(
      WiFi.macAddress());

  // =================================================
  // FORCE ESP-NOW CHANNEL
  // =================================================

  Serial.printf(
      "[WIFI] Setting channel to %d...\n",
      ESPNOW_CHANNEL);

  esp_wifi_set_promiscuous(true);

  esp_err_t channelResult =
      esp_wifi_set_channel(
          ESPNOW_CHANNEL,
          WIFI_SECOND_CHAN_NONE);

  esp_wifi_set_promiscuous(false);

  if (channelResult == ESP_OK)
  {
    Serial.println(
        "[WIFI] Channel configured successfully");
  }
  else
  {
    Serial.printf(
        "[ERROR] Channel configuration failed: %s\n",
        esp_err_to_name(channelResult));
  }

  // Get actual radio channel

  uint8_t primaryChannel;
  wifi_second_chan_t secondaryChannel;

  esp_wifi_get_channel(
      &primaryChannel,
      &secondaryChannel);

  Serial.printf(
      "[WIFI] Actual radio channel: %d\n",
      primaryChannel);

  // =================================================
  // CHANNEL CHECK
  // =================================================

  if (primaryChannel == ESPNOW_CHANNEL)
  {
    Serial.println(
        "[WIFI] CHANNEL CHECK: PASS");
  }
  else
  {
    Serial.println(
        "[WIFI] CHANNEL CHECK: FAILED");

    Serial.printf(
        "[WIFI] Expected: %d\n",
        ESPNOW_CHANNEL);

    Serial.printf(
        "[WIFI] Actual: %d\n",
        primaryChannel);
  }

  // =================================================
  // ESP-NOW INITIALIZATION
  // =================================================

  Serial.println(
      "[ESPNOW] Initializing...");

  esp_err_t result =
      esp_now_init();

  if (result != ESP_OK)
  {
    Serial.printf(
        "[ERROR] ESP-NOW init failed: %s\n",
        esp_err_to_name(result));

    while (true)
    {
      delay(1000);
    }
  }

  Serial.println(
      "[ESPNOW] Initialized successfully");

  // =================================================
  // REGISTER SEND CALLBACK
  // =================================================

  esp_now_register_send_cb(
      OnDataSent);

  Serial.println(
      "[ESPNOW] Send callback registered");

  // =================================================
  // CONFIGURE MOTHER PEER
  // =================================================

  memset(
      &peerInfo,
      0,
      sizeof(peerInfo));

  memcpy(
      peerInfo.peer_addr,
      receiverMAC,
      6);

  peerInfo.channel =
      ESPNOW_CHANNEL;

  peerInfo.encrypt = false;

  Serial.println();
  Serial.println(
      "[ESPNOW] Mother configuration:");

  Serial.printf(
      "[ESPNOW] Mother MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
      receiverMAC[0],
      receiverMAC[1],
      receiverMAC[2],
      receiverMAC[3],
      receiverMAC[4],
      receiverMAC[5]);

  Serial.printf(
      "[ESPNOW] Mother channel: %d\n",
      peerInfo.channel);

  // =================================================
  // ADD MOTHER AS PEER
  // =================================================

  result =
      esp_now_add_peer(
          &peerInfo);

  if (result != ESP_OK)
  {
    Serial.printf(
        "[ERROR] Failed to add Mother peer: %s\n",
        esp_err_to_name(result));

    while (true)
    {
      delay(1000);
    }
  }

  Serial.println(
      "[ESPNOW] Mother peer added successfully");

  // =================================================
  // READY
  // =================================================

  Serial.println();
  Serial.println(
      "========================================");

  Serial.println(
      "       DAUGHTER NODE READY");

  Serial.println(
      "========================================");

  Serial.printf(
      "GPIO %d -> INTRUSION\n",
      INTRUSION_BUTTON_PIN);

  Serial.printf(
      "GPIO %d -> SOS\n",
      SOS_BUTTON_PIN);

  Serial.printf(
      "ESP-NOW CHANNEL -> %d\n",
      ESPNOW_CHANNEL);

  Serial.println(
      "========================================");
}

// =====================================================
// LOOP
// =====================================================
void loop()
{
  // =================================================
  // READ BUTTONS
  // =================================================

  bool intrusionState =
      digitalRead(
          INTRUSION_BUTTON_PIN);

  bool sosState =
      digitalRead(
          SOS_BUTTON_PIN);

  // =================================================
  // INTRUSION BUTTON
  // =================================================

  if (intrusionState == LOW &&
      lastIntrusionState == HIGH)
  {
    Serial.println();
    Serial.println(
        "[BUTTON] INTRUSION PRESSED");

    sendMessage(
        MSG_INTRUSION,
        "INTRUSION");

    delay(200); // Debounce
  }

  // =================================================
  // SOS BUTTON
  // =================================================

  if (sosState == LOW &&
      lastSOSState == HIGH)
  {
    Serial.println();
    Serial.println(
        "[BUTTON] SOS PRESSED");

    sendMessage(
        MSG_SOS,
        "SOS");

    delay(200); // Debounce
  }

  // =================================================
  // SAVE BUTTON STATES
  // =================================================

  lastIntrusionState =
      intrusionState;

  lastSOSState =
      sosState;

  delay(10);
}