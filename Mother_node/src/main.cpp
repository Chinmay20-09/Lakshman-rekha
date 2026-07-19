#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <PubSubClient.h>

#define BUZZER_PIN 26

// WiFi Credentials
const char *ssid = "SSmhaskar_0879";
const char *password = "2131415161";

// MQTT Settings
const char *mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char *mqttTopic = "lakshmanrekha/alerts";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

typedef struct struct_message
{
  int value;
} struct_message;

struct_message incomingData;

void connectWiFi();
void connectMQTT();

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingDataBytes, int len)
{
  if (len != sizeof(struct_message))
  {
    Serial.println("Invalid data size");
    return;
  }

  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  Serial.print("Received: ");
  Serial.println(incomingData.value);

  if (incomingData.value == 1)
  {
    Serial.println("Publishing intrusion");

    if (mqttClient.connected())
    {
      mqttClient.publish(mqttTopic, "intrusion");
    }

    digitalWrite(BUZZER_PIN, HIGH);
    delay(2000);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void connectWiFi()
{
  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT... ");

    if (mqttClient.connect("MotherESP32"))
    {
      Serial.println("Connected!");
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 2 sec...");
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  connectWiFi();

  mqttClient.setServer(mqttServer, mqttPort);
  connectMQTT();

  Serial.print("Mother MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Mother Ready");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  if (!mqttClient.connected())
  {
    connectMQTT();
  }

  mqttClient.loop();
}