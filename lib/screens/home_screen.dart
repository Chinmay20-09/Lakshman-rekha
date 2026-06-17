import 'package:flutter/material.dart';
import '../models/app_state.dart';
import '../widgets/status_card.dart';
import '../theme/app_theme.dart';
import 'package:vibration/vibration.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen>
    with SingleTickerProviderStateMixin {
  late final AnimationController _pulseController;
  late final Animation<double> _pulseAnimation;

  @override
  void initState() {
    super.initState();
    _pulseController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 900),
    )..repeat(reverse: true);
    _pulseAnimation = Tween<double>(begin: 1.0, end: 1.08).animate(
      CurvedAnimation(parent: _pulseController, curve: Curves.easeInOut),
    );
    AppState().addListener(_onStateChange);
  }

  Future<void> triggerAlertVibration() async {
    if (await Vibration.hasVibrator() && await Vibration.hasCustomVibrationsSupport()) {
      Vibration.vibrate(
        pattern: [0, 500, 200, 500],
        repeat: 0, // repeat from index 0
      );
    }
  }

  void _onStateChange() async {
  if (!mounted) return;

  setState(() {});

  if (!AppState().isSafe) {
    await triggerAlertVibration();

    if (!mounted) return;

    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: const Text('Emergency alert delivered. Stay calm.'),
        backgroundColor: AppTheme.alertRed,
        action: SnackBarAction(
          label: 'Resolve',
          textColor: Colors.white,
          onPressed: () {
            Vibration.cancel();
            AppState().resolveAlerts();
          },
        ),
      ),
    );
  }
}

  void resolveAlerts() {
    Vibration.cancel();
    // existing resolve logic
  }

  @override
  void dispose() {
    AppState().removeListener(_onStateChange);
    _pulseController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final state = AppState();
    final theme = Theme.of(context);
    final statusColor = state.isSafe ? AppTheme.accentSafe : AppTheme.alertRed;

    return Scaffold(
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 18),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          'Hello, Chinmay',
                          style: theme.textTheme.headlineMedium,
                        ),
                        const SizedBox(height: 6),
                        Text(
                          'Your safe perimeter is active.',
                          style: theme.textTheme.bodyMedium,
                        ),
                      ],
                    ),
                  ),
                  GestureDetector(
                    onTap: () {
                      ScaffoldMessenger.of(context).showSnackBar(
                        SnackBar(
                          content: Text(
                            state.isDeviceOnline
                                ? 'Device connected'
                                : 'Device offline',
                          ),
                        ),
                      );
                    },
                    child: CircleAvatar(
                      radius: 24,
                      backgroundColor: state.isDeviceOnline
                          ? AppTheme.accentSafe
                          : Colors.grey.shade700,
                      child: Icon(
                        state.isDeviceOnline
                            ? Icons.wifi_outlined
                            : Icons.wifi_off,
                        color: Colors.white,
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 22),
              Row(
                children: [
                  Expanded(
                    child: StatusCard(
                      title: 'Security',
                      subtitle: state.isSafe ? 'SAFE' : 'ALERT',
                      icon: state.isSafe
                          ? Icons.check_circle_outline
                          : Icons.priority_high,
                      isPositive: state.isSafe,
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: StatusCard(
                      title: 'Device',
                      subtitle: state.isDeviceOnline ? 'ONLINE' : 'OFFLINE',
                      icon: state.isDeviceOnline ? Icons.wifi : Icons.wifi_off,
                      isPositive: state.isDeviceOnline,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 30),
              Expanded(
                child: Center(
                  child: ScaleTransition(
                    scale: _pulseAnimation,
                    child: GestureDetector(
                      onTap: () => state.triggerSOS(),
                      child: Container(
                        width: 220,
                        height: 220,
                        decoration: BoxDecoration(
                          shape: BoxShape.circle,
                          gradient: RadialGradient(
                            colors: [
                              statusColor,
                              statusColor.withAlpha(199),
                              Colors.black87,
                            ],
                            center: Alignment.center,
                            radius: 0.8,
                          ),
                          boxShadow: [
                            BoxShadow(
                              color: statusColor.withAlpha(102),
                              blurRadius: 32,
                              spreadRadius: 2,
                            ),
                          ],
                        ),
                        child: Center(
                          child: Column(
                            mainAxisSize: MainAxisSize.min,
                            children: [
                              const Icon(
                                Icons.sos_rounded,
                                size: 72,
                                color: Colors.white,
                              ),
                              const SizedBox(height: 10),
                              Text(
                                'Tap to alert',
                                style: theme.textTheme.headlineMedium?.copyWith(
                                  color: Colors.white,
                                  letterSpacing: 1.1,
                                ),
                              ),
                            ],
                          ),
                        ),
                      ),
                    ),
                  ),
                ),
              ),
              const SizedBox(height: 18),
              Container(
                padding: const EdgeInsets.all(18),
                decoration: BoxDecoration(
                  color: theme.cardColor,
                  borderRadius: BorderRadius.circular(20),
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black.withAlpha(13),
                      blurRadius: 18,
                      offset: const Offset(0, 10),
                    ),
                  ],
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('Quick actions', style: theme.textTheme.titleLarge),
                    const SizedBox(height: 16),
                    Row(
                      children: [
                        Expanded(
                          child: OutlinedButton.icon(
                            onPressed: state.simulateAlert,
                            icon: const Icon(Icons.flash_on),
                            label: const Text('Simulate Alert'),
                            style: OutlinedButton.styleFrom(
                              padding: const EdgeInsets.symmetric(vertical: 16),
                              shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(16),
                              ),
                            ),
                          ),
                        ),
                        const SizedBox(width: 12),
                        Expanded(
                          child: OutlinedButton.icon(
                            onPressed: () => state.toggleDeviceStatus(),
                            icon: const Icon(Icons.sensors),
                            label: const Text('Check Sensors'),
                            style: OutlinedButton.styleFrom(
                              padding: const EdgeInsets.symmetric(vertical: 16),
                              shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(16),
                              ),
                            ),
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
