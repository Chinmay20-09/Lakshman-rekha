#include <Arduino.h>

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define BUTTON_PIN 27
#define ESPNOW_CHANNEL 10

uint8_t receiverMAC[] = {
    0xB0, 0xCB, 0xD8, 0xC6, 0xA5, 0xA8}; // Mother node MAC

typedef struct struct_message
{
  int value;
} struct_message;

struct_message outgoingData;

esp_now_peer_info_t peerInfo;
const unsigned long SOS_HOLD_TIME = 3000;
bool buttonPressed = false;
bool sosSentThisPress = false;
unsigned long pressStart = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.println();
  Serial.println("[SEND] +++++ ESP-NOW Send Callback +++++");
  Serial.printf("[SEND] Receiver MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("[SEND] Status: SUCCESS - packet delivered");
  }
  else
  {
    Serial.println("[SEND] Status: FAILED - packet not delivered");
  }
  Serial.println("[SEND] ----- End of Callback -----");
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("[BOOT] ========================================");
  Serial.println("[BOOT] Daughter Node Booting...");
  Serial.println("[BOOT] ========================================");
  Serial.printf("[BOOT] Firmware: Daughter Node v1.0\r\n");
  Serial.printf("[BOOT] Compiled: %s %s\r\n", __DATE__, __TIME__);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("[BOOT] Button pin initialized (INPUT_PULLUP on GPIO 27)");

  WiFi.mode(WIFI_STA);
  Serial.println("[WIFI] WiFi set to STA mode");

  // Force WiFi to a fixed channel
  Serial.printf("[WIFI] Forcing channel to %d...\r\n", ESPNOW_CHANNEL);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  Serial.printf("[WIFI] Channel set to %d\r\n", ESPNOW_CHANNEL);
  Serial.printf("[WIFI] Actual WiFi Channel: %d\r\n", WiFi.channel());
  Serial.printf("[WIFI] MAC Address: %s\r\n", WiFi.macAddress().c_str());

  Serial.println("[BOOT] Stage 1/2: Initializing ESP-NOW...");
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

  esp_now_register_send_cb(OnDataSent);
  Serial.println("[ESPNOW] Send callback registered");

  Serial.println("[BOOT] Stage 2/2: Adding peer...");
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = ESPNOW_CHANNEL; // Fixed channel
  peerInfo.encrypt = false;

  Serial.printf("[ESPNOW] Peer MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                peerInfo.peer_addr[0], peerInfo.peer_addr[1],
                peerInfo.peer_addr[2], peerInfo.peer_addr[3],
                peerInfo.peer_addr[4], peerInfo.peer_addr[5]);
  Serial.printf("[ESPNOW] Peer Channel: %d\r\n", peerInfo.channel);
  Serial.printf("[ESPNOW] Peer Encrypt: %s\r\n", peerInfo.encrypt ? "Yes" : "No");

  ret = esp_now_add_peer(&peerInfo);
  if (ret != ESP_OK)
  {
    Serial.printf("[ERROR] Failed to Add Peer: %s (0x%x)\r\n", esp_err_to_name(ret), ret);
    return;
  }
  else
  {
    Serial.println("[ESPNOW] Peer added successfully");
  }

  Serial.println("[BOOT] ----------------------------------------");
  Serial.println("[BOOT] Daughter Node Ready");
  Serial.printf("[BOOT] Button: GPIO%d (press to send)\r\n", BUTTON_PIN);
  Serial.printf("[BOOT] Target MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                receiverMAC[0], receiverMAC[1], receiverMAC[2],
                receiverMAC[3], receiverMAC[4], receiverMAC[5]);
  Serial.printf("[BOOT] ESP-NOW Channel: %d\r\n", ESPNOW_CHANNEL);
  Serial.println("[BOOT] ----------------------------------------");
  Serial.println();
}



void loop()
{
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);

  // --- Button just pressed: start timing ---
  if (pressed && !buttonPressed)
  {
    buttonPressed = true;
    sosSentThisPress = false;
    pressStart = millis();

    Serial.println("[BUTTON] Button PRESSED (GPIO 27 -> LOW)");
  }

  // --- Long-press threshold reached: send SOS exactly once ---
  if (pressed && buttonPressed && !sosSentThisPress && (millis() - pressStart >= SOS_HOLD_TIME))
  {
    sosSentThisPress = true;
    outgoingData.value = 2;

    Serial.printf("[SEND] Preparing SOS packet: value=%d, size=%d bytes\r\n",
                  outgoingData.value, sizeof(outgoingData));

    esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&outgoingData, sizeof(outgoingData));
    Serial.printf("[SEND] esp_now_send() returned: %s (0x%x)\r\n",
                  (result == ESP_OK) ? "ESP_OK" : esp_err_to_name(result), result);

    if (result != ESP_OK)
    {
      Serial.println("[ERROR] SOS send failed! Check peer status and channel.");
    }
    Serial.println("[SEND] SOS packet sent, waiting for callback...");
  }

  // --- Button released ---
  if (!pressed && buttonPressed)
  {
    buttonPressed = false;

    if (sosSentThisPress)
    {
      // Was a long press — SOS already sent above, nothing more to do
      Serial.println("[BUTTON] Button released after SOS");
    }
    else
    {
      // Short press (< 3 s) — send intrusion
      outgoingData.value = 1;

      Serial.printf("[SEND] Preparing intrusion packet: value=%d, size=%d bytes\r\n",
                    outgoingData.value, sizeof(outgoingData));

      esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&outgoingData, sizeof(outgoingData));
      Serial.printf("[SEND] esp_now_send() returned: %s (0x%x)\r\n",
                    (result == ESP_OK) ? "ESP_OK" : esp_err_to_name(result), result);

      if (result != ESP_OK)
      {
        Serial.println("[ERROR] Send failed! Check peer status and channel.");
      }
      Serial.println("[SEND] Intrusion packet sent, waiting for callback...");
    }
  }
}
