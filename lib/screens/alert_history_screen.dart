import 'package:flutter/material.dart';
import '../models/app_state.dart';
import '../theme/app_theme.dart';

class AlertHistoryScreen extends StatefulWidget {
  const AlertHistoryScreen({super.key});

  @override
  State<AlertHistoryScreen> createState() => _AlertHistoryScreenState();
}

class _AlertHistoryScreenState extends State<AlertHistoryScreen> {
  @override
  void initState() {
    super.initState();
    AppState().addListener(_onStateChange);
  }

  void _onStateChange() {
    if (mounted) setState(() {});
  }

  @override
  void dispose() {
    AppState().removeListener(_onStateChange);
    super.dispose();
  }

  String _formatTimestamp(DateTime timestamp) {
    final now = DateTime.now();
    final diff = now.difference(timestamp);
    if (diff.inDays == 0) return 'Today · ${timestamp.hour.toString().padLeft(2,'0')}:${timestamp.minute.toString().padLeft(2,'0')}';
    if (diff.inDays == 1) return 'Yesterday · ${timestamp.hour.toString().padLeft(2,'0')}:${timestamp.minute.toString().padLeft(2,'0')}';
    return '${timestamp.day}/${timestamp.month}/${timestamp.year}';
  }

  @override
  Widget build(BuildContext context) {
    final alerts = AppState().alerts;
    final theme = Theme.of(context);

    return Scaffold(
      appBar: AppBar(title: const Text('Alert History')),
      body: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 14),
        child: alerts.isEmpty
            ? Center(child: Text('No alerts recorded.', style: theme.textTheme.bodyLarge))
            : GridView.builder(
                gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
                  crossAxisCount: 1,
                  mainAxisExtent: 100,
                  childAspectRatio: 5,
                ),
                itemCount: alerts.length,
                itemBuilder: (context, index) {
                  final a = alerts[index];
                  final color = a.resolved ? AppTheme.accentSafe : AppTheme.alertRed;
                  return Card(
                    shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
                    elevation: 3,
                    child: Padding(
                      padding: const EdgeInsets.all(12.0),
                      child: Row(
                        children: [
                          Container(
                            width: 56,
                            height: 56,
                            decoration: BoxDecoration(
                              color: color.withAlpha(36),
                              borderRadius: BorderRadius.circular(12),
                            ),
                            child: Icon(
                              a.resolved ? Icons.check_circle : Icons.warning_amber_rounded,
                              color: color,
                            ),
                          ),
                          const SizedBox(width: 12),
                          Expanded(
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              mainAxisAlignment: MainAxisAlignment.center,
                              children: [
                                Text(a.title, style: theme.textTheme.titleMedium?.copyWith(fontWeight: FontWeight.w700)),
                                const SizedBox(height: 6),
                                Text(_formatTimestamp(a.timestamp), style: theme.textTheme.bodySmall),
                              ],
                            ),
                          ),
                          Text(a.type.toUpperCase(), style: theme.textTheme.labelLarge?.copyWith(color: color, fontWeight: FontWeight.w700)),
                        ],
                      ),
                    ),
                  );
                },
              ),
      ),
    );
  }
}
