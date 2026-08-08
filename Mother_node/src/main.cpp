#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <PubSubClient.h>

// -------------------- Pins --------------------
#define BUZZER_PIN 26 #include<Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// =====================================================
// BUTTON PINS
// =====================================================

#define INTRUSION_BUTTON_PIN 27
#define SOS_BUTTON_PIN 25

// =====================================================
// ESP-NOW CHANNEL
// =====================================================

#define ESPNOW_CHANNEL 10

// =====================================================
// MOTHER NODE MAC ADDRESS
// =====================================================

uint8_t receiverMAC[] = {
    0xB0, 0xCB, 0xD8,
    0xC6, 0xA5, 0xA8};

// =====================================================
// ESP-NOW MESSAGE
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
// SEND CALLBACK
// =====================================================

void OnDataSent(
    const uint8_t *mac_addr,
    esp_now_send_status_t status)
{
  Serial.println();
  Serial.println("[ESPNOW] ===== SEND CALLBACK =====");
  
  Serial.printf(
      "[ESPNOW] Receiver: %02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
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
// SEND MESSAGE
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

  Serial.println(
      "[WIFI] Mode: STA");

  Serial.println(
      "[WIFI] Sleep: DISABLED");

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
      "[ESPNOW] Mother MAC: %02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
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

    // value = 1

    sendMessage(
        1,
        "INTRUSION");

    // Debounce

    delay(200);
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

    // value = 2

    sendMessage(
        2,
        "SOS");

    // Debounce

    delay(200);
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
#define ESPNOW_CHANNEL 10
// -------------------- WiFi Credentials --------------------
const char *ssid = "SSmhaskar_0879";
const char *password = "2131415161";

// -------------------- MQTT --------------------
const char *mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char *mqttTopic = "lakshmanrekha/alerts";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// -------------------- ESP-NOW Message --------------------
typedef struct struct_message
{
  int value;
} struct_message;

struct_message incomingData;

// ---------------------------------------------------------

void connectWiFi()
{
  Serial.println();
  Serial.println("[WIFI] Connecting to WiFi...");
  Serial.printf("[WIFI] SSID: %s\r\n", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[WIFI] WiFi Connected!");
  Serial.printf("[WIFI] IP Address: %s\r\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WIFI] WiFi Channel: %d\r\n", WiFi.channel());
  Serial.printf("[WIFI] MAC Address: %s\r\n", WiFi.macAddress().c_str());
}

void connectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.printf("[MQTT] Connecting to broker at %s:%d...\r\n", mqttServer, mqttPort);

    if (mqttClient.connect("MotherESP32"))
    {
      Serial.println("[MQTT] Connected to broker successfully!");
      Serial.printf("[MQTT] Subscribed topic: %s\r\n", mqttTopic);
    }
    else
    {
      int rc = mqttClient.state();
      Serial.printf("[MQTT] Connection failed, rc=%d\r\n", rc);
      switch (rc)
      {
        case -4: Serial.println("[MQTT] Reason: MQTT_CONNECTION_TIMEOUT"); break;
        case -3: Serial.println("[MQTT] Reason: MQTT_CONNECTION_LOST"); break;
        case -2: Serial.println("[MQTT] Reason: MQTT_CONNECT_FAILED"); break;
        case -1: Serial.println("[MQTT] Reason: MQTT_DISCONNECTED"); break;
        case 1:  Serial.println("[MQTT] Reason: MQTT_CONNECT_BAD_PROTOCOL"); break;
        case 2:  Serial.println("[MQTT] Reason: MQTT_CONNECT_BAD_CLIENT_ID"); break;
        case 3:  Serial.println("[MQTT] Reason: MQTT_CONNECT_UNAVAILABLE"); break;
        case 4:  Serial.println("[MQTT] Reason: MQTT_CONNECT_BAD_CREDENTIALS"); break;
        case 5:  Serial.println("[MQTT] Reason: MQTT_CONNECT_UNAUTHORIZED"); break;
        default: Serial.println("[MQTT] Reason: UNKNOWN"); break;
      }
      Serial.println("[MQTT] Retrying in 2 seconds...");
      delay(2000);
    }
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingDataBytes, int len)
{
  Serial.println();
  Serial.println("[RECV] +++++ ESP-NOW Packet Received +++++");
  Serial.printf("[RECV] Sender MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("[RECV] Packet Length: %d bytes\r\n", len);
  Serial.printf("[RECV] Expected Size: %d bytes\r\n", sizeof(struct_message));

  if (len != sizeof(struct_message))
  {
    Serial.println("[ERROR] Invalid packet size - discarding");
    return;
  }

  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  Serial.printf("[RECV] Received Value: %d\r\n", incomingData.value);

  if (incomingData.value == 1)
  {
    Serial.println("[RECV] Intrusion Detected! Activating buzzer and publishing MQTT...");

    if (mqttClient.connected())
    {
      Serial.printf("[MQTT] Publishing to topic '%s': intrusion\r\n", mqttTopic);
      if (mqttClient.publish(mqttTopic, "intrusion"))
      {
        Serial.println("[MQTT] Published successfully");
      }
      else
      {
        Serial.println("[ERROR] MQTT publish failed");
      }
    }
    else
    {
      Serial.println("[ERROR] MQTT client not connected - cannot publish");
    }

    Serial.println("[RECV] Buzzer ON (2000ms)");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(2000);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("[RECV] Buzzer OFF");
  }
  else if (incomingData.value == 2)
  {
    Serial.println("[RECV] SOS Received!");

    if (mqttClient.connected())
    {
      Serial.printf("[MQTT] Publishing to topic '%s': sos\r\n", mqttTopic);
      if (mqttClient.publish(mqttTopic, "sos"))
      {
        Serial.println("[MQTT] Published successfully");
      }
      else
      {
        Serial.println("[ERROR] MQTT publish failed");
      }
    }
    else
    {
      Serial.println("[ERROR] MQTT client not connected - cannot publish");
    }

    Serial.println("[RECV] Buzzer ON (2000ms)");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(2000);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("[RECV] Buzzer OFF");
  }
  else
  {
    Serial.printf("[RECV] Ignoring unknown value (%d)\r\n", incomingData.value);
  }

  Serial.println("[RECV] ----- End of Packet -----");
  Serial.println();
}
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("[BOOT] ========================================");
  Serial.println("[BOOT] Mother Node Booting...");
  Serial.println("[BOOT] ========================================");
  Serial.printf("[BOOT] Firmware: Mother Node v1.0\r\n");
  Serial.printf("[BOOT] Compiled: %s %s\r\n", __DATE__, __TIME__);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("[BOOT] Buzzer pin initialized (LOW)");

  // Connect WiFi
  Serial.println("[BOOT] Stage 1/3: Connecting to WiFi...");
  connectWiFi();

  // MQTT
  Serial.println("[BOOT] Stage 2/3: Connecting to MQTT...");
  mqttClient.setServer(mqttServer, mqttPort);
  connectMQTT();

  // ESP-NOW
  Serial.println("[BOOT] Stage 3/3: Initializing ESP-NOW...");
  esp_err_t ret = esp_now_init();
  if (ret != ESP_OK)
  {
    Serial.printf("[ERROR] ESP-NOW Init Failed: %s (0x%x)\r\n", esp_err_to_name(ret), ret);
    return;
  }
  else
  {
    Serial.println("[ESPNOW] ESP-NOW initialized successfully");
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("[ESPNOW] Receive callback registered");

  Serial.println("[BOOT] ----------------------------------------");
  Serial.println("[BOOT] Mother Node Ready");
  Serial.printf("[BOOT] Listening on channel %d\r\n", ESPNOW_CHANNEL);
  Serial.println("[BOOT] ----------------------------------------");
  Serial.println();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WIFI] Connection lost! Reconnecting...");
    connectWiFi();
  }

  if (!mqttClient.connected())
  {
    Serial.println("[MQTT] Connection lost! Reconnecting...");
    connectMQTT();
  }

  mqttClient.loop();
}