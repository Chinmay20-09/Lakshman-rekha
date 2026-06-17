# Lakshman Rekha

A personal safety and emergency-response system built with **Flutter** and **ESP32**, designed to provide fast SOS activation, real-time status monitoring, and emergency communication when the user is in distress.

---

## Overview

Lakshman Rekha combines a mobile application and a wearable/hardware trigger to create a reliable emergency alert system.

The Flutter application serves as the control center, while the ESP32-based device can trigger emergency events even when accessing the phone directly is difficult.

The system is designed with a simple goal:

> **Trigger help as quickly as possible with minimal user interaction.**

---

## Features

### Mobile Application (Flutter)

* Clean and modern UI
* Emergency SOS activation
* Real-time safety status dashboard
* MQTT communication with ESP32
* Vibration feedback for emergency events
* Device connection monitoring
* Emergency contact management (planned)
* Location sharing (planned)
* Background emergency monitoring (planned)

### ESP32 Hardware

* Physical emergency trigger
* MQTT-based communication
* Low-latency alert transmission
* Battery-powered deployment support
* Expandable sensor integration

---

## Tech Stack

### Frontend

* Flutter
* Dart
* Material Design

### Communication

* MQTT Protocol

### Hardware

* ESP32

### Planned Services

* Firebase (Authentication)
* Firebase Cloud Messaging (Push Notifications)
* Location Services
* Emergency Contact Integration

---

## System Architecture

```text
┌─────────────┐
│   ESP32     │
│ SOS Trigger │
└──────┬──────┘
       │ MQTT
       ▼
┌─────────────────┐
│ MQTT Broker     │
└──────┬──────────┘
       │
       ▼
┌─────────────────┐
│ Flutter App     │
│ Lakshman Rekha  │
└──────┬──────────┘
       │
       ▼
┌─────────────────┐
│ Emergency Logic │
└─────────────────┘
```

---

## Project Structure

```text
lib/
├── models/
│   └── app_state.dart
│
├── screens/
│   ├── home_screen.dart
│   └── ...
│
├── widgets/
│   ├── status_card.dart
│   └── ...
│
├── theme/
│   └── app_theme.dart
│
└── main.dart
```

---

## Getting Started

### Prerequisites

Install:

* Flutter SDK
* Android Studio or VS Code
* Dart SDK
* ESP32 Development Environment
* MQTT Broker (Mosquitto recommended)

---

### Clone the Repository

```bash
git clone https://github.com/your-username/lakshman-rekha.git

cd lakshman-rekha
```

---

### Install Dependencies

```bash
flutter pub get
```

---

### Run the Application

```bash
flutter run
```

---

## MQTT Configuration

Configure the broker settings inside the application:

```dart
const String broker = "mqtt://your-broker-address";
const String topic = "lakshman_rekha/sos";
```

### Example Message

```json
{
  "device": "esp32",
  "event": "SOS",
  "timestamp": "2026-06-18T12:00:00Z"
}
```

---

## Current Development Status

### Completed

* Flutter project setup
* Safety dashboard UI
* App state management
* Vibration feedback
* MQTT architecture decision
* ESP32 communication design

### In Progress

* MQTT integration
* ESP32 firmware
* Connection status handling

### Planned

* Authentication
* GPS location sharing
* Emergency contact notifications
* SMS fallback system
* Push notifications
* Cloud dashboard
* Incident history

---

## Roadmap

### Phase 1 — MVP

* [x] Flutter UI
* [x] State management
* [x] SOS workflow design
* [ ] MQTT communication
* [ ] ESP32 trigger integration

### Phase 2 — Emergency Response

* [ ] Live location sharing
* [ ] Emergency contacts
* [ ] Push notifications
* [ ] Background monitoring

### Phase 3 — Production

* [ ] Cloud backend
* [ ] Analytics
* [ ] Multi-device support
* [ ] Battery optimization
* [ ] Security hardening

---

## Safety Disclaimer

Lakshman Rekha is a supplementary safety tool and should not be considered a replacement for emergency services, law enforcement, or professional safety systems. Users should always contact local emergency authorities whenever possible.

---

## Contributing

Contributions, suggestions, and bug reports are welcome.

1. Fork the repository
2. Create a feature branch

```bash
git checkout -b feature/new-feature
```

3. Commit changes

```bash
git commit -m "Add new feature"
```

4. Push to your branch

```bash
git push origin feature/new-feature
```

5. Open a Pull Request

---

## License

This project is licensed under the MIT License.

---

### Vision

**"A simple physical action should be enough to call for help when every second matters."**
