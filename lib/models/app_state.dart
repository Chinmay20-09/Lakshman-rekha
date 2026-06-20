import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'dart:convert';

/// Global alert level for the application UI
enum AlertLevel { none, alert, sos }

class Contact {
  final String id;
  final String name;
  final String phone;

  Contact({required this.id, required this.name, required this.phone});

  Map<String, dynamic> toJson() => {'id': id, 'name': name, 'phone': phone};

  factory Contact.fromJson(Map<String, dynamic> json) => Contact(
    id: json['id'] as String,
    name: json['name'] as String,
    phone: json['phone'] as String,
  );
}

class AlertEvent {
  final String id;
  final String type; // e.g., intrusion, fire, gas, device_offline, sos
  final String title; // human friendly message
  final DateTime timestamp;
  final bool resolved;
  // Future fields for richer telemetry
  // String? zone;
  // String? deviceName;
  // String? sensorId;

  AlertEvent({
    required this.id,
    required this.type,
    required this.title,
    required this.timestamp,
    this.resolved = false,
  });

  Map<String, dynamic> toJson() => {
    'id': id,
    'type': type,
    'title': title,
    'timestamp': timestamp.toIso8601String(),
    'resolved': resolved,
  };

  factory AlertEvent.fromJson(Map<String, dynamic> json) => AlertEvent(
    id: json['id'] as String,
    type: json['type'] as String? ?? json['title'] as String? ?? 'unknown',
    title: json['title'] as String? ?? json['type'] as String? ?? 'Alert',
    timestamp: DateTime.parse(json['timestamp'] as String),
    resolved: json['resolved'] as bool? ?? false,
  );
}

class AppState extends ChangeNotifier {
  static final AppState _instance = AppState._internal();
  factory AppState() => _instance;
  AppState._internal();

  // Current global alert level and message for UI-driven alerts.
  AlertLevel currentAlertLevel = AlertLevel.none;
  String alertMessage = '';

  static const String _contactsKey = 'app_contacts';
  static const String _alertsKey = 'app_alerts';

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

  /// Load persisted data from local storage. Call once during app startup.
  Future<void> load() async {
    try {
      final prefs = await SharedPreferences.getInstance();
      final contactsJson = prefs.getString(_contactsKey);
      if (contactsJson != null) {
        final List<dynamic> data = jsonDecode(contactsJson) as List<dynamic>;
        _contacts
          ..clear()
          ..addAll(
            data.map((e) => Contact.fromJson(e as Map<String, dynamic>)),
          );
      }

      final alertsJson = prefs.getString(_alertsKey);
      if (alertsJson != null) {
        final List<dynamic> data = jsonDecode(alertsJson) as List<dynamic>;
        _alerts = data
            .map((e) => AlertEvent.fromJson(e as Map<String, dynamic>))
            .toList();
      }
    } catch (e) {
      // If persistence fails, keep in-memory defaults; do not crash the app.
      // Optionally log the error in a real app.
    }
    notifyListeners();
  }

  Future<void> _saveContacts() async {
    try {
      final prefs = await SharedPreferences.getInstance();
      final jsonStr = jsonEncode(_contacts.map((c) => c.toJson()).toList());
      await prefs.setString(_contactsKey, jsonStr);
    } catch (e) {
      // ignore write errors for now
    }
  }

  Future<void> _saveAlerts() async {
    try {
      final prefs = await SharedPreferences.getInstance();
      final jsonStr = jsonEncode(_alerts.map((a) => a.toJson()).toList());
      await prefs.setString(_alertsKey, jsonStr);
    } catch (e) {
      // ignore write errors for now
    }
  }

  void triggerSOS() {
    _isSafe = false;
    currentAlertLevel = AlertLevel.sos;
    alertMessage = 'SOS Activated';
    _addAlert(type: 'sos', title: 'SOS triggered from app', level: AlertLevel.sos);
  }

  void simulateAlert() {
    _isSafe = false;
    currentAlertLevel = AlertLevel.alert;
    alertMessage = 'Demo sensor breach detected';
    _addAlert(type: 'intrusion', title: 'Demo sensor breach detected', level: AlertLevel.alert);
  }

  void resolveAlerts() {
    _isSafe = true;
    currentAlertLevel = AlertLevel.none;
    alertMessage = '';
    _alerts = _alerts
        .map(
          (alert) => AlertEvent(
            id: alert.id,
            type: alert.type,
            title: alert.title,
            timestamp: alert.timestamp,
            resolved: true,
          ),
        )
        .toList();
    _saveAlerts();
    notifyListeners();
  }

  void toggleTheme() {
    _isDarkMode = !_isDarkMode;
    notifyListeners();
  }

  // Generic helper used internally to create and record alert events.
  // Future: map MQTT payloads to these helper calls and enrich with zone/device metadata.
  void _addAlert({required String type, required String title, required AlertLevel level}) {
    final event = AlertEvent(
      id: DateTime.now().toIso8601String(),
      type: type,
      title: title,
      timestamp: DateTime.now(),
      resolved: false,
    );

    _alerts.insert(0, event);
    // Update UI signaling
    currentAlertLevel = level;
    alertMessage = title;
    _saveAlerts();
    notifyListeners();
  }

  // High level alert triggers for different event types
  void triggerIntrusionAlert({String? message}) {
    _isSafe = false;
    _addAlert(type: 'intrusion', title: message ?? 'Intrusion detected', level: AlertLevel.alert);
  }

  void triggerFireAlert({String? message}) {
    _isSafe = false;
    _addAlert(type: 'fire', title: message ?? 'Fire detected', level: AlertLevel.alert);
  }

  void triggerGasAlert({String? message}) {
    _isSafe = false;
    _addAlert(type: 'gas', title: message ?? 'Gas leak detected', level: AlertLevel.alert);
  }

  void triggerDeviceOfflineAlert({String? message}) {
    _isDeviceOnline = false;
    _addAlert(type: 'device_offline', title: message ?? 'Device offline', level: AlertLevel.alert);
  }

  void triggerSOSAlert({String? message}) {
    _isSafe = false;
    _addAlert(type: 'sos', title: message ?? 'SOS Activated', level: AlertLevel.sos);
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
    _saveContacts();
    notifyListeners();
  }

  void removeContact(String id) {
    _contacts.removeWhere((contact) => contact.id == id);
    _saveContacts();
    notifyListeners();
  }
}
