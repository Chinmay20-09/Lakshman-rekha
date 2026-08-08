# Lakshman Rekha — Visual Guide

> **How to view this file:** Mermaid diagrams render natively on **GitHub**, **GitLab**, and in **VS Code** (with the *Markdown Preview Mermaid Support* extension). Quoted subgraph titles and `style` lines need Mermaid ≥ 9.2 (any current GitHub/VS Code extension qualifies). ASCII fallbacks are provided for the two most important diagrams in case Mermaid is unavailable.
>
> All diagrams reflect the **current source** (`Daughter_node/src/main.cpp`, `Mother_node/src/main.cpp`). Source is authoritative — if a diagram disagrees with code, trust the code.

---

## Table of Contents

1. [System Context](#1-system-context)
2. [High-Level Architecture](#2-high-level-architecture)
3. [Node Responsibilities](#3-node-responsibilities)
4. [Project Tree](#4-project-tree)
5. [Hardware Wiring](#5-hardware-wiring)
6. [Alert Data Flow](#6-alert-data-flow)
7. [End-to-End Sequence (Intrusion)](#7-end-to-end-sequence-intrusion)
8. [Message Protocol](#8-message-protocol)
9. [ESP-NOW Channel Alignment](#9-esp-now-channel-alignment)
10. [Boot Sequences](#10-boot-sequences)
11. [ESP-NOW Reliability & Retry](#11-esp-now-reliability--retry)
12. [MQTT Bridge](#12-mqtt-bridge)
13. [Troubleshooting Decision Tree](#13-troubleshooting-decision-tree)
14. [Future: Multiple Daughter Nodes](#14-future-multiple-daughter-nodes)
15. [Quick Reference Map](#15-quick-reference-map)

---

## 1. System Context

```mermaid
flowchart LR
    subgraph LOCAL["🔒 Local (no internet needed)"]
        B1["Intrusion Button<br/>GPIO 27"] --> D["Daughter ESP32<br/>ESP-NOW TX"]
        B2["SOS Button<br/>GPIO 25"] --> D
        D -->|"ESP-NOW, channel 2"| M["Mother ESP32<br/>ESP-NOW RX + WiFi + MQTT"]
        M -->|"alert"| BZ["Buzzer GPIO 26"]
    end

    subgraph CLOUD["☁️ Internet"]
        MQ["MQTT Broker<br/>broker.emqx.io : 1883"]
        F["Flutter Application"]
    end

    M -->|"Wi-Fi"| MQ
    MQ -->|"subscribe"| F

    style LOCAL fill:#eef7ff
    style CLOUD fill:#eaffea
```

> The critical path **Button → Daughter → Mother → Buzzer** uses only ESP-NOW and works even if Wi-Fi/internet are down. MQTT adds the cloud + Flutter leg.

---

## 2. High-Level Architecture

```mermaid
flowchart LR
    A["Daughter ESP32<br/>**buttons → ESP-NOW**"] -->|"value = 1 or 2"| B["Mother ESP32<br/>**ESP-NOW → MQTT bridge**"]
    B -->|"publish"| C["MQTT Broker<br/>topic: lakshmanrekha/alerts"]
    C -->|"subscribe"| D["Flutter App"]
    B -->|"beep"| E["Buzzer"]
```

**ASCII fallback:**

```
 Daughter ESP32                 Mother ESP32                   MQTT Broker                Flutter App
+--------------+    ESP-NOW    +---------------+   Wi-Fi    +--------------------+  sub   +---------+
| GPIO 27 → 1  | ────────────▶ |  ESP-NOW RX   | ─────────▶ | broker.emqx.io     | ──────▶ |  alerts |
| GPIO 25 → 2  |  channel 2    |  + WiFi + MQTT|  publish   | lakshmanrekha/alerts|        +---------+
+--------------+               +-------+-------+            +--------------------+
                                       │
                                       ▼
                                   [Buzzer GPIO 26]
```

---

## 3. Node Responsibilities

```mermaid
flowchart TB
    subgraph DUTY["Daughter — sensor node"]
        A1["Reads buttons (GPIO 27, 25)"]
        A2["Encodes alert: 1 or 2"]
        A3["Sends via ESP-NOW<br/>3 attempts max"]
        A4["❌ No MQTT, no WiFi join"]
    end

    subgraph MOT["Mother — bridge node"]
        B1["Receives ESP-NOW packets"]
        B2["Decodes value → intrusion / sos"]
        B3["Activates buzzer (500 ms)"]
        B4["Joins WiFi + publishes MQTT"]
    end

    A1 --> A2 --> A3
    B1 --> B2 --> B3
    B2 --> B4

    style DUTY fill:#fff3e0
    style MOT fill:#e8f5e9
```

| Capability | Daughter | Mother |
|---|---|---|
| Buttons | ✅ | ❌ |
| ESP-NOW transmit | ✅ | ❌ |
| ESP-NOW receive | ❌ | ✅ |
| Buzzer | ❌ | ✅ (GPIO 26) |
| Wi-Fi / MQTT | ❌ | ✅ |
| Internet bridge | ❌ | ✅ |

---

## 4. Project Tree

```mermaid
graph TD
    ROOT["PlatformIO-Projects/  (repo root)"]
    ROOT --> RD["README.md"]
    ROOT --> VG["VISUAL_GUIDE.md  ← you are here"]
    ROOT --> EX["EXECUTIVE_SUMMARY_ESP-NOW_SEND_FAIL.md"]
    ROOT --> VS[".vscode/"]
    ROOT --> DN["Daughter_node/"]
    DN --> DNP["platformio.ini<br/>esp32dev, COM5, 115200 baud"]
    DN --> DNS["src/main.cpp<br/>buttons → ESP-NOW TX"]
    DN --> DND["LAKSHMAN_REKHA_ENGINEERING_DOCUMENT.md"]
    ROOT --> MN["Mother_node/"]
    MN --> MNP["platformio.ini<br/>esp32dev, COM5, PubSubClient"]
    MN --> MNS["src/main.cpp<br/>ESP-NOW RX → WiFi → MQTT"]
```

**ASCII fallback:**

```
PlatformIO-Projects/
├── README.md
├── VISUAL_GUIDE.md                 ← this file
├── EXECUTIVE_SUMMARY_ESP-NOW_SEND_FAIL.md
├── .vscode/
├── Daughter_node/
│   ├── platformio.ini              (esp32dev · COM5 · monitor 115200)
│   ├── src/main.cpp                (buttons → ESP-NOW transmitter)
│   └── LAKSHMAN_REKHA_ENGINEERING_DOCUMENT.md
└── Mother_node/
    ├── platformio.ini              (esp32dev · COM5 · PubSubClient@^2.8)
    └── src/main.cpp                (ESP-NOW receiver → Wi-Fi → MQTT)
```

> **Note:** both projects use `upload_port = COM5` — flash one board at a time or change the port for the second board.

---

## 5. Hardware Wiring

```mermaid
flowchart LR
    subgraph DAUGHTER["Daughter ESP32"]
        P27["GPIO 27"] ---|"button → GND"| IB["Intrusion Button"]
        P25["GPIO 25"] ---|"button → GND"| SB["SOS Button"]
        G1["GND"] --- IB
        G1 --- SB
    end

    subgraph MOTHER["Mother ESP32"]
        P26["GPIO 26"] ---|"signal"| BZ["Buzzer"]
    end

    DAUGHTER -. "ESP-NOW / 2.4 GHz" .-> MOTHER
```

| Device | GPIO | Function | Logic | Wiring |
|---|---|---|---|---|
| Daughter | 27 | Intrusion button | `INPUT_PULLUP`, **active-low** | GPIO 27 → button → GND |
| Daughter | 25 | SOS button | `INPUT_PULLUP`, **active-low** | GPIO 25 → button → GND |
| Mother | 26 | Buzzer | active-high output | GPIO 26 → buzzer → GND |

**Button polarity (INPUT_PULLUP):**

```mermaid
flowchart LR
    UP["Pin HIGH = released<br/>(internal pull-up to 3.3 V)"] --- DN["Pin LOW = pressed<br/>(shorted to GND)"]
```

> Both buttons trigger only on the **falling edge** (`LOW` after previously `HIGH`), so one press = one event. A `delay(200)` debounce follows each send.

---

## 6. Alert Data Flow

### Intrusion

```mermaid
flowchart LR
    B["Intrusion button GPIO 27"] -->|"LOW"| D["sendMessage(1, 'INTRUSION')"]
    D -->|"ESP-NOW frame value=1"| M["Mother OnDataRecv"]
    M -->|"pendingEvent = true"| L["loop() → processPendingEvent()"]
    L -->|"case 1"| BZ["Buzzer ON 500 ms"]
    L -->|"publish 'intrusion'"| Q["MQTT topic lakshmanrekha/alerts"]
    Q --> F["Flutter"]
```

### SOS

```mermaid
flowchart LR
    B["SOS button GPIO 25"] -->|"LOW"| D["sendMessage(2, 'SOS')"]
    D -->|"ESP-NOW frame value=2"| M["Mother OnDataRecv"]
    M -->|"pendingEvent = true"| L["loop() → processPendingEvent()"]
    L -->|"case 2"| BZ["Buzzer ON 500 ms"]
    L -->|"publish 'sos'"| Q["MQTT topic lakshmanrekha/alerts"]
    Q --> F["Flutter"]
```

---

## 7. End-to-End Sequence (Intrusion)

```mermaid
sequenceDiagram
    autonumber
    actor User
    participant D as Daughter ESP32
    participant M as Mother ESP32
    participant Q as MQTT Broker
    participant F as Flutter App

    User->>D: Press GPIO 27 (intrusion)
    D->>D: sendMessage(1, "INTRUSION")<br/>attempt 1/3
    D->>M: ESP-NOW frame (value=1) over channel 2
    M-->>D: Link-layer ACK
    Note over D: OnDataSent → STATUS: SUCCESS<br/>"Delivery confirmed (ACK)"
    M->>M: OnDataRecv → pendingEvent = true
    M->>M: loop() → processPendingEvent()
    M->>Q: publish "intrusion" → lakshmanrekha/alerts
    Q-->>F: delivers "intrusion"
    F->>F: show INTRUSION alert
    M->>M: buzzer ON 500 ms → OFF
```

> If the Mother never ACKs, the Daughter retries (see [Section 11](#11-esp-now-reliability--retry)).

---

## 8. Message Protocol

```mermaid
flowchart LR
    subgraph STRUCT["struct_message { int value; }"]
        V["value = 1"]
        W["value = 2"]
    end

    V -->|"Daughter sends"| M1["Mother case 1"]
    M1 --> P1["MQTT payload 'intrusion'"]
    W -->|"Daughter sends"| M2["Mother case 2"]
    M2 --> P2["MQTT payload 'sos'"]
```

| Value | Meaning | Daughter action | Mother action | MQTT payload |
|---|---|---|---|---|
| `1` | Intrusion | `sendMessage(MSG_INTRUSION, "INTRUSION")` | `case 1:` → buzzer + publish | `intrusion` |
| `2` | SOS | `sendMessage(MSG_SOS, "SOS")` | `case 2:` → buzzer + publish | `sos` |

> The struct must stay **byte-identical on both nodes** — the Mother rejects packets whose size ≠ `sizeof(struct_message)`.

---

## 9. ESP-NOW Channel Alignment

```mermaid
flowchart LR
    R["Router / AP<br/>2.4 GHz channel"] -->|"locks the radio"| M["Mother ESP32 radio<br/>= AP channel"]
    R -->|"must be equal"| D["Daughter ESP32 radio<br/>ESPNOW_CHANNEL = 2 (forced)"]
    M <-->|"ESP-NOW works only when equal"| D

    style D fill:#fff3e0
    style M fill:#e8f5e9
```

**The rule:**

```
Router/AP channel  =  Mother radio channel  =  Daughter ESP-NOW channel (2)
```

- The **Daughter** hard-forces channel **2** with the promiscuous trick and verifies it with `esp_wifi_get_channel()` (`CHANNEL CHECK: PASS/FAILED`).
- The **Mother's** `#define ESPNOW_CHANNEL 10` is **label-only** — its radio follows the Wi-Fi AP. It prints `Actual Wi-Fi Channel: N` at boot.
- **Current consequence:** your router must be on **channel 2**, or every send ends in `STATUS: FAILED`.

```mermaid
flowchart LR
    A["Daughter forces channel 2"] --> B{"Mother's Actual Wi-Fi Channel?"}
    B -->|"2"| OK["✅ ESP-NOW works"]
    B -->|"1 / 6 / 11 / other"| FAIL["❌ No ACK → STATUS: FAILED<br/>fix: set router to channel 2"]
```

---

## 10. Boot Sequences

### Daughter boot (state diagram)

```mermaid
stateDiagram-v2
    [*] --> Serial: Serial.begin(115200)
    Serial --> Buttons: pinMode(27,25) INPUT_PULLUP
    Buttons --> WiFi: WiFi.mode(WIFI_STA)
    WiFi --> Sleep: WiFi.setSleep(false)
    Sleep --> BT: btStop()
    BT --> Channel: force channel 2 (promiscuous trick)
    Channel --> Check: esp_wifi_get_channel()
    Check --> Init: esp_now_init()
    Init --> CB: register send callback
    CB --> Peer: esp_now_add_peer(Mother MAC)
    Peer --> Ready: DAUGHTER NODE READY
    Init --> Halt: init failed
    Peer --> Halt2: add peer failed
    Halt --> [*]
    Halt2 --> [*]
```

### Mother boot (state diagram)

```mermaid
stateDiagram-v2
    [*] --> Serial: Serial.begin(115200)
    Serial --> Buzzer: pinMode(26) OUTPUT LOW
    Buzzer --> Sleep: WiFi.setSleep(false)
    Sleep --> Mac: print Mother MAC
    Mac --> WiFi: connectWiFi() (15 s timeout)
    WiFi --> Chan: print Actual Wi-Fi Channel
    Chan --> Init: esp_now_init()
    Init --> CB: register receive callback
    CB --> MQTT: setServer + connectMQTT() (10 s timeout)
    MQTT --> Ready: MOTHER NODE READY
    MQTT --> Failed: MQTT: FAILED (loop retries)
```

---

## 11. ESP-NOW Reliability & Retry

### `esp_now_send()` vs callback verdict

```mermaid
sequenceDiagram
    participant D as Daughter
    participant N as ESP-NOW stack
    participant M as Mother

    D->>N: esp_now_send()
    Note over N: returns ESP_OK = "accepted for TX"<br/>(NOT delivery)
    N->>M: radio frame
    M-->>N: link-layer ACK
    N-->>D: OnDataSent callback
    Note over D: ESP_NOW_SEND_SUCCESS = delivered<br/>ESP_NOW_SEND_FAIL = no ACK
```

### Current retry flow (Daughter `sendMessage()`)

```mermaid
flowchart TD
    A["Button press (falling edge)"] --> B["attempt = 1"]
    B --> C{"esp_now_send()<br/>returned ESP_OK?"}
    C -- "No" --> C1["Print 'Failed to start transmission'"]
    C1 --> STOP["⛔ Stop - no retry"]
    C -- "Yes" --> D["delay(100) - wait for callback"]
    D --> E{"Callback:<br/>lastSendFailed == false?"}
    E -- "Yes" --> F["Print 'Delivery confirmed (ACK)'"]
    F --> DONE["✅ Done"]
    E -- "No" --> G{"attempt < 3?"}
    G -- "Yes" --> H["attempt + 1<br/>print 'no ACK, retrying...'"]
    H --> C
    G -- "No" --> I["Print 'Delivery failed after 3 attempts'"]
    I --> DONE
```

| Knob | Current value |
|---|---|
| Max attempts | `3` |
| Wait per attempt | `delay(100)` (blocking) |
| Extra retry gap | none (the 100 ms wait doubles as it) |
| Success print | `[SEND] Delivery confirmed (ACK)` |
| Exhaustion print | `[SEND] ERROR: Delivery failed after 3 attempts` |
| Application-level ACK | ❌ **not implemented** — "ACK" is only the Wi-Fi link-layer ACK |

---

## 12. MQTT Bridge

```mermaid
sequenceDiagram
    participant D as Daughter
    participant M as Mother ESP32
    participant Q as broker.emqx.io:1883
    participant F as Flutter App

    D->>M: ESP-NOW value 1
    M->>M: processPendingEvent()<br/>case 1 → payload "intrusion"
    M->>Q: publish "intrusion" → lakshmanrekha/alerts
    Q-->>F: "intrusion"

    D->>M: ESP-NOW value 2
    M->>M: case 2 → payload "sos"
    M->>Q: publish "sos" → lakshmanrekha/alerts
    Q-->>F: "sos"
```

```mermaid
flowchart LR
    M["Mother"] -->|"Wi-Fi<br/>SSID: SSmhaskar_0879"| R["Router / AP"]
    R -->|"internet"| B["broker.emqx.io : 1883<br/>topic lakshmanrekha/alerts"]
    B --> F["Flutter app subscribes"]
```

> **Security:** plaintext port 1883 on a public broker; `peerInfo.encrypt = false` — prototype only (see README §19).

---

## 13. Troubleshooting Decision Tree

```mermaid
flowchart TD
    S["Button press → 'STATUS: FAILED'?"] --> Q1{"Daughter boot:<br/>CHANNEL CHECK: PASS?"}
    Q1 -- "FAILED" --> F1["Router/AP not on channel 2<br/>→ set router to channel 2"]
    Q1 -- "PASS" --> Q2{"Mother boot:<br/>Actual Wi-Fi Channel = 2?"}
    Q2 -- "No" --> F2["Channel mismatch<br/>→ fix router channel"]
    Q2 -- "Yes" --> Q3{"Daughter's 'Mother MAC'<br/>matches Mother's print?"}
    Q3 -- "No" --> F3["Update receiverMAC[] in Daughter"]
    Q3 -- "Yes" --> Q4{"Mother powered and flashed?"}
    Q4 -- "No" --> F4["Power/flash Mother first"]
    Q4 -- "Yes" --> Q5{"Mother prints 'MQTT: CONNECTED'?"}
    Q5 -- "No" --> F5["Check Wi-Fi, internet, port 1883, client ID 'MotherESP32'"]
    Q5 -- "Yes" --> F6["Interference / distance / power<br/>→ move closer, try another channel, stable supply"]
```

**Quick symptom map:**

| Symptom | Look at | Likely cause |
|---|---|---|
| `STATUS: FAILED` always | both `CHANNEL CHECK` lines | channel mismatch |
| `STATUS: FAILED` sometimes | distance / power | RF range or supply |
| No `[ESPNOW]` line on Mother | Mother channel print | router channel ≠ 2 |
| `MQTT: FAILED` | rc codes (`-2`, `5`, …) | broker unreachable / ID clash |
| No `[BUTTON]` print | wiring | button not LOW on press |
| Wrong alert shown | struct + cases | value/payload mapping drift |

---

## 14. Future: Multiple Daughter Nodes

```mermaid
flowchart TB
    M["Mother ESP32 (1 receiver)"]
    M --> D1["Daughter 1<br/>MAC unique · channel 2"]
    M --> D2["Daughter 2<br/>MAC unique · channel 2"]
    M --> D3["Daughter 3<br/>MAC unique · channel 2"]
    M --> DN["Daughter N<br/>MAC unique · channel 2"]

    style M fill:#e8f5e9
```

- All Daughters share the **same channel** and the **same Mother MAC**.
- The Mother currently accepts any sender; sender identity exists only via the optional `expectedDaughterMAC` (supports one, default `00:00:00:00:00:00` = disabled).
- A future struct could add `device_id`, `event_type`, `timestamp`, `battery` — **this is future work; the current struct is `{ int value; }` only.**

```mermaid
flowchart LR
    NOW["Now: struct_message { int value; }"] --> FUT["Future: { device_id, event_type, timestamp, battery, … }"]
    style NOW fill:#fff3e0
    style FUT fill:#e8f5e9
```

---

## 15. Quick Reference Map

```mermaid
flowchart LR
    subgraph D["DAUGHTER"]
        D1["Role: buttons → ESP-NOW TX"]
        D2["Mother MAC: B0:CB:D8:C6:A5:A8"]
        D3["ESP-NOW channel: 2 (forced)"]
        D4["GPIO 27 = intrusion · GPIO 25 = SOS"]
        D5["Retries: 3 × 100 ms"]
    end

    subgraph M["MOTHER"]
        M1["Role: ESP-NOW RX → MQTT bridge"]
        M2["Wi-Fi: SSMhaskar_0879 (password: SECRET)"]
        M3["MQTT: broker.emqx.io:1883"]
        M4["Topic: lakshmanrekha/alerts"]
        M5["Buzzer GPIO 26 (500 ms)"]
    end

    subgraph MSG["MESSAGES"]
        V1["1 = intrusion → 'intrusion'"]
        V2["2 = sos → 'sos'"]
    end

    D --> MSG --> M
```

| Item | Value |
|---|---|
| Daughter channel | `2` (hardcoded, forced) |
| Mother MAC (target) | `B0:CB:D8:C6:A5:A8` |
| MQTT broker / port / topic | `broker.emqx.io` / `1883` / `lakshmanrekha/alerts` |
| Payloads | `intrusion` (value 1), `sos` (value 2) |
| Serial baud / monitor | `115200` |
| Build / flash / monitor | `pio run` · `pio run --target upload` · `pio device monitor` |

---

*Generated from the current source. If you change firmware, re-check the diagrams — especially [§9 channel](#9-esp-now-channel-alignment) and [§8 message values](#8-message-protocol).*
