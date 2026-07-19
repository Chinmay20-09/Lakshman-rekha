#include <Arduino.h>

#include <esp_now.h>
#include <WiFi.h>

#define BUTTON_PIN 27

uint8_t receiverMAC[] = {
    0xB0, 0xCB, 0xD8, 0xC6, 0xA5, 0xA8};//mother node mac address
typedef struct struct_message
{
  int value;
} struct_message;

struct_message outgoingData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Message Sent");
  }
  else
  {
    Serial.println("Send Failed");
  }
}

void setup()
{

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  Serial.print("Daughter MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to Add Peer");
    return;
  }

  Serial.println("Daughter Ready");
}

void loop()
{

  if (digitalRead(BUTTON_PIN) == LOW)
  {

    outgoingData.value = 1;

    esp_now_send(receiverMAC, (uint8_t *)&outgoingData, sizeof(outgoingData));

    delay(500);
  }
}