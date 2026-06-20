import 'package:flutter/material.dart';
import '../models/app_state.dart';
import 'package:vibration/vibration.dart';

class SosActiveScreen extends StatefulWidget {
  const SosActiveScreen({super.key});

  @override
  State<SosActiveScreen> createState() => _SosActiveScreenState();
}

class _SosActiveScreenState extends State<SosActiveScreen>
    with TickerProviderStateMixin {
  late final AnimationController _flashController;
  late final Animation<Color?> _colorAnim;
  late final AnimationController _holdController;

  @override
  void initState() {
    super.initState();
    _flashController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 700),
    )..repeat(reverse: true);

    _colorAnim = ColorTween(
      begin: Colors.red.shade900,
      end: Colors.red.shade400,
    ).animate(_flashController);

    _holdController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 3),
    );

    // Start continuous vibration if available. Keep short pattern and repeat.
    Vibration.hasVibrator().then((has) {
      if (has == true) {
        Vibration.vibrate(pattern: [0, 700, 300], repeat: 0);
      }
    });
  }

  @override
  void dispose() {
    Vibration.cancel();
    _flashController.dispose();
    _holdController.dispose();
    super.dispose();
  }

  void _onLongPressStart(LongPressStartDetails _) {
    // Start filling the progress indicator
    _holdController.forward().then((_) {
      if (_holdController.status == AnimationStatus.completed) {
        // Completed hold -> stop vibration, mark resolved, and close
        Vibration.cancel();
        AppState().resolveAlerts();
        if (mounted) Navigator.of(context).pop();
      }
    });
  }

  void _onLongPressEnd(LongPressEndDetails _) {
    // Reset if not completed
    if (_holdController.status != AnimationStatus.completed) {
      _holdController.reset();
    }
  }

  @override
  Widget build(BuildContext context) {
    return PopScope(
      child: AnimatedBuilder(
        animation: _colorAnim,
        builder: (context, child) {
          return Scaffold(
            backgroundColor: _colorAnim.value,
            body: SafeArea(
              child: Center(
                child: Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 28.0),
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      const Icon(
                        Icons.warning_amber_rounded,
                        size: 72,
                        color: Colors.white,
                      ),
                      const SizedBox(height: 18),
                      const Text(
                        '🚨 SOS ACTIVE',
                        style: TextStyle(
                          color: Colors.white,
                          fontSize: 32,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: 12),
                      const Text(
                        'Emergency assistance has been requested',
                        style: TextStyle(color: Colors.white70, fontSize: 18),
                        textAlign: TextAlign.center,
                      ),
                      const SizedBox(height: 30),
                      GestureDetector(
                        onLongPressStart: _onLongPressStart,
                        onLongPressEnd: _onLongPressEnd,
                        child: Stack(
                          alignment: Alignment.center,
                          children: [
                            SizedBox(
                              width: 140,
                              height: 140,
                              child: AnimatedBuilder(
                                animation: _holdController,
                                builder: (context, child) {
                                  return CircularProgressIndicator(
                                    value: _holdController.value,
                                    color: Colors.white,
                                    backgroundColor: Colors.white24,
                                    strokeWidth: 8,
                                  );
                                },
                              ),
                            ),
                            const Text(
                              'Hold 3 sec to Exit',
                              style: TextStyle(
                                color: Colors.white,
                                fontWeight: FontWeight.w700,
                              ),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          );
        },
      ),
    );
  }
}
