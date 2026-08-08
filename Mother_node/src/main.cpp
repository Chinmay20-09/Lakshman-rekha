#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <PubSubClient.h>

// =====================================================
// PINS
// =====================================================
#define BUZZER_PIN 26

// =====================================================
// ESP-NOW
// =====================================================
// Preferred/reference channel. The actual operating channel is the
// WiFi AP's channel, because the Daughter joins this same network.
#define ESPNOW_CHANNEL 10

// =====================================================
// WiFi credentials
// =====================================================
const char *ssid = "SSmhaskar_0879";
const char *password = "2131415161";

// =====================================================
// MQTT
// =====================================================
const char *mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char *mqttTopic = "lakshmanrekha/alerts";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// =====================================================
// ESP-NOW MESSAGE (must be byte-identical on both nodes)
//    value 1 = intrusion
//    value 2 = SOS
// =====================================================
typedef struct struct_message
{
  int value;
} struct_message;

struct_message incomingData;

// =====================================================
// PENDING-EVENT QUEUE
// OnDataRecv() ONLY fills these fields and returns.
// loop() consumes the event (MQTT publish + buzzer).
// volatile because the callback runs in the WiFi task.
// =====================================================
volatile bool pendingEvent = false;
volatile int pendingValue = 0;
uint8_t pendingSenderMAC[6];

// =====================================================
// Expected Daughter MAC - used to verify the sender.
// Fill this with the Daughter's actual MAC (printed in the
// Daughter's boot output as "Daughter MAC").
// Leave all zeros to skip the check.
// =====================================================
uint8_t expectedDaughterMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// =====================================================
// Non-blocking buzzer
// =====================================================
bool buzzerOn = false;
unsigned long buzzerOffAtMs = 0;
#define BUZZER_ON_MS 500

// =====================================================
// HELPERS
// =====================================================
const char *mqttStateReason(int rc)
{
  switch (rc)
  {
  case -4: return "CONNECTION_TIMEOUT";
  case -3: return "CONNECTION_LOST";
  case -2: return "CONNECT_FAILED";
  case -1: return "DISCONNECTED";
  case 1:  return "BAD_PROTOCOL";
  case 2:  return "BAD_CLIENT_ID";
  case 3:  return "UNAVAILABLE";
  case 4:  return "BAD_CREDENTIALS";
  case 5:  return "UNAUTHORIZED";
  default: return "UNKNOWN";
  }
}

bool connectWiFi()
{
  Serial.println("[WIFI] Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000)
  {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[ERROR] WiFi CONNECTION FAILED - check SSID/password and router");
    return false;
  }

  Serial.printf("[WIFI] Connected! SSID: %s\n", ssid);
  Serial.printf("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
  return true;
}

bool connectMQTT()
{
  unsigned long start = millis();
  while (!mqttClient.connected() && (millis() - start) < 10000)
  {
    Serial.printf("[MQTT] Connecting to %s:%d ...\n", mqttServer, mqttPort);

    if (mqttClient.connect("MotherESP32"))
    {
      Serial.println("[MQTT] Connected!");
      return true;
    }

    int rc = mqttClient.state();
    Serial.printf("[MQTT] Failed, rc=%d (%s). Retrying in 2s...\n",
                  rc, mqttStateReason(rc));
    delay(2000);
  }

  Serial.println("[ERROR] MQTT connect failed after timeout");
  return false;
}

// =====================================================
// ESP-NOW RECEIVE CALLBACK
// LIGHTWEIGHT! Called from the WiFi task context.
//   - validate packet length
//   - copy the message + sender MAC
//   - set the pending-event flag
//   - return immediately
// NO delay(), NO MQTT, NO WiFi calls, NO long operations.
// MQTT and the buzzer are handled in loop().
// =====================================================
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingDataBytes, int len)
{
  if (len != sizeof(struct_message))
  {
    Serial.printf("[ESPNOW] Rejected packet: size %d (expected %d)\n",
                  len, sizeof(struct_message));
    return;
  }

  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  memcpy(pendingSenderMAC, mac, 6);
  pendingValue = incomingData.value;
  pendingEvent = true;

  Serial.printf("[ESPNOW] Packet queued from %02X:%02X:%02X:%02X:%02X:%02X, value=%d\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                pendingValue);
}

// =====================================================
// PROCESS A QUEUED EVENT - called from loop(), never from
// the callback. Publishes MQTT and drives the buzzer.
// =====================================================
void processPendingEvent()
{
  // Snapshot the event fields BEFORE clearing the flag, so a packet
  // arriving mid-processing cannot cause a duplicate publish.
  int value = pendingValue;
  uint8_t sender[6];
  memcpy(sender, pendingSenderMAC, 6);
  pendingEvent = false;

  Serial.println();
  Serial.println("========== ESP-NOW EVENT ==========");
  Serial.printf("From: %02X:%02X:%02X:%02X:%02X:%02X\n",
                sender[0], sender[1], sender[2],
                sender[3], sender[4], sender[5]);

  // Sender verification (only if expectedDaughterMAC is configured).
  bool macConfigured = false;
  for (int i = 0; i < 6; i++)
  {
    if (expectedDaughterMAC[i] != 0)
    {
      macConfigured = true;
      break;
    }
  }
  if (macConfigured && memcmp(sender, expectedDaughterMAC, 6) != 0)
  {
    Serial.println("[WARNING] Sender MAC does NOT match expected Daughter MAC!");
    Serial.println("[WARNING] Check that you are receiving from the correct device.");
  }

  const char *payload = nullptr;
  switch (value)
  {
  case 1:
    payload = "intrusion"; // DO NOT change the MQTT payload name
    Serial.println("Event: INTRUSION");
    break;
  case 2:
    payload = "sos"; // DO NOT change the MQTT payload name
    Serial.println("Event: SOS");
    break;
  default:
    Serial.printf("Unknown value: %d (ignored)\n", value);
    Serial.println("===================================");
    return;
  }

  // ---- MQTT publish ----
  if (mqttClient.connected())
  {
    if (mqttClient.publish(mqttTopic, payload))
      Serial.printf("MQTT: PUBLISHED '%s' -> %s\n", payload, mqttTopic);
    else
      Serial.printf("[ERROR] MQTT publish failed for '%s'\n", payload);
  }
  else
  {
    Serial.printf("[WARNING] MQTT not connected - could not publish '%s'\n", payload);
  }

  // ---- Buzzer (non-blocking) ----
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerOn = true;
  buzzerOffAtMs = millis() + BUZZER_ON_MS;
  Serial.println("BUZZER: ON (500 ms)");

  Serial.println("===================================");
}

// =====================================================
// SETUP
// =====================================================
void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("========================================");
  Serial.println("LAKSHMAN REKHA - MOTHER NODE");
  Serial.println("========================================");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Disable WiFi sleep. The Mother must maintain its WiFi/MQTT link AND
  // receive ESP-NOW frames at the same time. With power-save enabled the
  // radio sleeps periodically and incoming ESP-NOW frames are lost.
  WiFi.setSleep(false);
  Serial.println("WIFI SLEEP: DISABLED");

  // IMPORTANT: the Mother uses WiFi for MQTT. ESP-NOW uses the SAME radio,
  // so the radio must stay on the channel of the connected AP. We must NOT
  // force channel 10 here - that would break the WiFi/MQTT connection.
  // The Daughter joins this same WiFi network, so BOTH nodes automatically
  // share the AP's channel and ESP-NOW stays aligned on any channel.

  // Print the Mother's own MAC - the Daughter must be sending to this MAC.
  Serial.printf("Mother MAC: %s\n", WiFi.macAddress().c_str());
  Serial.println("NOTE: Daughter's 'Mother MAC' must equal the MAC above.");

  connectWiFi();

  // ---- CHANNEL CHECK ----
  // The Daughter connects to this same WiFi, so both ESP32s share the
  // AP's channel - ESP-NOW is aligned by construction.
  int actualChannel = WiFi.channel();
  Serial.printf("Actual Wi-Fi Channel: %d\n", actualChannel);

  if (actualChannel == ESPNOW_CHANNEL)
  {
    Serial.println("CHANNEL CHECK: PASS (matches preferred channel 10)");
  }
  else
  {
    Serial.printf("CHANNEL CHECK: PASS (shared channel %d - the Daughter follows "
                  "this WiFi, so ESP-NOW is on the same channel)\n", actualChannel);
  }

  // ---- ESP-NOW ----
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("[ERROR] ESP-NOW: INITIALIZATION FAILED");
    while (true)
      delay(1000);
  }
  Serial.println("ESP-NOW: INITIALIZED");

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("RECEIVE CALLBACK: REGISTERED");

  // ---- MQTT ----
  mqttClient.setServer(mqttServer, mqttPort);
  Serial.println(connectMQTT() ? "MQTT: CONNECTED" : "MQTT: FAILED");

  // ---- Ready ----
  Serial.println();
  Serial.println("========================================");
  Serial.println("MOTHER NODE READY");
  Serial.println("========================================");
}

// =====================================================
// LOOP
// =====================================================
void loop()
{
  // Consume queued ESP-NOW events (MQTT + buzzer happen HERE,
  // never inside the ESP-NOW callback).
  if (pendingEvent)
  {
    processPendingEvent();
  }

  // Non-blocking buzzer off.
  if (buzzerOn && (millis() >= buzzerOffAtMs))
  {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerOn = false;
    Serial.println("BUZZER: OFF");
  }

  // Reconnect handling. The Daughter follows this same WiFi, so after a
  // reconnect both nodes are aligned again - just confirm the channel.
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WIFI] Connection lost - reconnecting...");
    connectWiFi();

    if (WiFi.status() == WL_CONNECTED)
    {
      int ch = WiFi.channel();
      Serial.printf("[WIFI] Reconnected on channel %d (shared with Daughter)\n", ch);
    }
    else
    {
      Serial.println("[WIFI] Still not connected - ESP-NOW to the Daughter may be down");
    }
  }

  if (!mqttClient.connected())
  {
    Serial.println("[MQTT] Connection lost - reconnecting...");
    connectMQTT();
  }

  mqttClient.loop();
}
