import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import '../models/app_state.dart';
import 'voice.dart';

class MQTTService {
  final TTSService tts = TTSService();
  MQTTService._();

  static final MQTTService instance = MQTTService._();

  late MqttServerClient client;

  Future<void> connect() async {
  debugPrint("MQTT: connect() called");

  try {
    client = MqttServerClient(
      'broker.emqx.io',
      'lakshman_rekha_flutter',
    );

    client.port = 1883;

    debugPrint("MQTT: attempting connection");

    await client.connect();

    debugPrint("MQTT: connected successfully");

    client.subscribe(
      'lakshmanrekha/alerts',
      MqttQos.atLeastOnce,
    );

    debugPrint("MQTT: subscribed");
    client.updates?.listen((messages) {
  debugPrint("MQTT: message arrived");

  final recMess =
      messages.first.payload as MqttPublishMessage;

  final payload =
      MqttPublishPayload.bytesToStringAsString(
    recMess.payload.message,
  );
  debugPrint("PAYLOAD RAW: [$payload]");
debugPrint("PAYLOAD LENGTH: ${payload.length}");

final cleanPayload = payload.trim().toLowerCase();

debugPrint("PAYLOAD CLEAN: [$cleanPayload]");

if (cleanPayload == "sos") {
  AppState().triggerSOSAlert();
   tts.speak("SOS activated");
}

if (cleanPayload == "intrusion") {
  AppState().triggerIntrusionAlert();
  tts.speak("Intrusion Alert.");
}

  
  debugPrint("MQTT RECEIVED: $payload");
});
  } catch (e) {
    debugPrint("MQTT ERROR: $e");
  }
}

}