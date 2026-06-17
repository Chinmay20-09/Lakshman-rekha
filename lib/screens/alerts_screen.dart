import 'package:flutter/material.dart';
import '../models/app_state.dart';
import '../theme/app_theme.dart';

class AlertsScreen extends StatefulWidget {
  const AlertsScreen({super.key});

  @override
  State<AlertsScreen> createState() => _AlertsScreenState();
}

class _AlertsScreenState extends State<AlertsScreen> {
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
    final difference = now.difference(timestamp);
    if (difference.inDays == 0) {
      return 'Today · ${timestamp.hour.toString().padLeft(2, '0')}:${timestamp.minute.toString().padLeft(2, '0')}';
    }
    if (difference.inDays == 1) {
      return 'Yesterday · ${timestamp.hour.toString().padLeft(2, '0')}:${timestamp.minute.toString().padLeft(2, '0')}';
    }
    return '${timestamp.day}/${timestamp.month}/${timestamp.year}';
  }

  @override
  Widget build(BuildContext context) {
    final alerts = AppState().alerts;
    final theme = Theme.of(context);

    return Scaffold(
      appBar: AppBar(title: const Text('Incident Alerts')),
      body: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Track recent system events',
              style: theme.textTheme.bodyLarge?.copyWith(
                color: theme.colorScheme.onSurface.withAlpha(178),
              ),
            ),
            const SizedBox(height: 18),
            Expanded(
              child: alerts.isEmpty
                  ? Center(
                      child: Text(
                        'No alerts yet. Your perimeter is quiet.',
                        style: theme.textTheme.bodyMedium,
                      ),
                    )
                  : ListView.separated(
                      itemCount: alerts.length,
                      separatorBuilder: (context, index) =>
                          const SizedBox(height: 14),
                      itemBuilder: (context, index) {
                        final alert = alerts[index];
                        final alertColor = alert.resolved
                            ? AppTheme.accentSafe
                            : AppTheme.alertRed;
                        return Card(
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(18),
                          ),
                          elevation: 3,
                          child: ListTile(
                            contentPadding: const EdgeInsets.symmetric(
                              horizontal: 18,
                              vertical: 16,
                            ),
                            leading: Container(
                              width: 48,
                              height: 48,
                              decoration: BoxDecoration(
                                color: alertColor.withAlpha(36),
                                borderRadius: BorderRadius.circular(14),
                              ),
                              child: Icon(
                                alert.resolved
                                    ? Icons.check_circle
                                    : Icons.warning_amber_rounded,
                                color: alertColor,
                                size: 28,
                              ),
                            ),
                            title: Text(
                              alert.title,
                              style: theme.textTheme.titleLarge?.copyWith(
                                fontWeight: FontWeight.w700,
                              ),
                            ),
                            subtitle: Padding(
                              padding: const EdgeInsets.only(top: 8),
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Text(
                                    _formatTimestamp(alert.timestamp),
                                    style: theme.textTheme.bodyMedium,
                                  ),
                                  const SizedBox(height: 6),
                                  Text(
                                    alert.resolved ? 'Resolved' : 'Active',
                                    style: theme.textTheme.bodySmall?.copyWith(
                                      color: alertColor,
                                      fontWeight: FontWeight.w600,
                                    ),
                                  ),
                                ],
                              ),
                            ),
                            trailing: alert.resolved
                                ? null
                                : IconButton(
                                    icon: Icon(
                                      Icons.check,
                                      color: theme.colorScheme.primary,
                                    ),
                                    onPressed: AppState().resolveAlerts,
                                  ),
                          ),
                        );
                      },
                    ),
            ),
          ],
        ),
      ),
    );
  }
}
