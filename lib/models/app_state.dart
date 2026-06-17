import 'package:flutter/material.dart';

class Contact {
  final String id;
  final String name;
  final String phone;

  Contact({required this.id, required this.name, required this.phone});
}

class AlertEvent {
  final String id;
  final String title;
  final DateTime timestamp;
  final bool resolved;

  AlertEvent({
    required this.id,
    required this.title,
    required this.timestamp,
    this.resolved = false,
  });
}

class AppState extends ChangeNotifier {
  static final AppState _instance = AppState._internal();
  factory AppState() => _instance;
  AppState._internal();

  bool _isSafe = true;
  bool _isDeviceOnline = true;
  bool _isDarkMode = false;
  bool _notificationsEnabled = true;
  bool _demoModeEnabled = false;

  final List<Contact> _contacts = [
    Contact(id: '1', name: 'Mom', phone: '+1 234 567 8900'),
    Contact(id: '2', name: 'Dad', phone: '+1 234 567 8901'),
  ];

  List<AlertEvent> _alerts = [];

  bool get isSafe => _isSafe;
  bool get isDeviceOnline => _isDeviceOnline;
  bool get isDarkMode => _isDarkMode;
  bool get notificationsEnabled => _notificationsEnabled;
  bool get demoModeEnabled => _demoModeEnabled;
  List<Contact> get contacts => List.unmodifiable(_contacts);
  List<AlertEvent> get alerts => List.unmodifiable(_alerts);

  void triggerSOS() {
    _isSafe = false;
    _alerts.insert(
      0,
      AlertEvent(
        id: DateTime.now().toIso8601String(),
        title: 'SOS triggered from app',
        timestamp: DateTime.now(),
        resolved: false,
      ),
    );
    notifyListeners();
  }

  void simulateAlert() {
    _isSafe = false;
    _alerts.insert(
      0,
      AlertEvent(
        id: DateTime.now().toIso8601String(),
        title: 'Demo sensor breach detected',
        timestamp: DateTime.now(),
        resolved: false,
      ),
    );
    notifyListeners();
  }

  void resolveAlerts() {
    _isSafe = true;
    _alerts = _alerts
        .map(
          (alert) => AlertEvent(
            id: alert.id,
            title: alert.title,
            timestamp: alert.timestamp,
            resolved: true,
          ),
        )
        .toList();
    notifyListeners();
  }

  void toggleTheme() {
    _isDarkMode = !_isDarkMode;
    notifyListeners();
  }

  void toggleNotifications() {
    _notificationsEnabled = !_notificationsEnabled;
    notifyListeners();
  }

  void toggleDeviceStatus() {
    _isDeviceOnline = !_isDeviceOnline;
    notifyListeners();
  }

  void toggleDemoMode() {
    _demoModeEnabled = !_demoModeEnabled;
    notifyListeners();
  }

  void addContact(String name, String phone) {
    _contacts.add(
      Contact(id: DateTime.now().toIso8601String(), name: name, phone: phone),
    );
    notifyListeners();
  }

  void removeContact(String id) {
    _contacts.removeWhere((contact) => contact.id == id);
    notifyListeners();
  }
}
