import 'package:flutter_tts/flutter_tts.dart';

class TTSService {
  final FlutterTts flutterTts = FlutterTts();

  Future<void> init() async {
    await flutterTts.setLanguage("en-IN");
    await flutterTts.setSpeechRate(0.5);
    await flutterTts.setPitch(1.0);
    await flutterTts.setVolume(1.0);

    // Wait until speech finishes
    await flutterTts.awaitSpeakCompletion(true);
  }

  Future speak(String text) async {
    await flutterTts.stop(); // Stop previous speech
    await flutterTts.speak(text);
  }

  Future stop() async {
    await flutterTts.stop();
  }
}