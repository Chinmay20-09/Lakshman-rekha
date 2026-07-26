#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <PubSubClient.h>

// -------------------- Pins --------------------
#define BUZZER_PIN 26
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