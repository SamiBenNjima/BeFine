# BeFine — Smart Inhaler Companion System

> **IoT · ESP32-S3 · Flutter · BLE 5.0 · FreeRTOS · Clean Architecture**

---

## What is BeFine?

BeFine is an open-source IoT medical device ecosystem designed to help patients use their metered-dose inhalers (MDIs) correctly. Incorrect inhaler technique is one of the leading causes of poor asthma and COPD outcomes — BeFine addresses this by embedding a sensor-rich microcontroller inside the inhaler attachment, pairing it wirelessly to a smartphone companion app, and guiding the patient step-by-step through the correct procedure while measuring and scoring their technique in real time.

---

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        BeFine Ecosystem                         │
│                                                                 │
│   ┌──────────────────────────┐      BLE 5.0 GATT               │
│   │   Smart Inhaler Device   │ ◄──────────────────────────►    │
│   │   (ESP32-S3 Firmware)    │      50 Hz flow stream          │
│   │                          │                                  │
│   │  • SDP31 flow sensor     │   ┌────────────────────────┐    │
│   │  • SHT41 temp/humidity   │   │   BeFine Flutter App   │    │
│   │  • MPU6050 IMU (shake)   │   │   (Android / iOS)      │    │
│   │  • QRE spray detector    │   │                         │    │
│   │  • SSD1306 OLED display  │   │  • Guided session UI   │    │
│   │  • WS2812B RGB LED       │   │  • Real-time flow chart│    │
│   │  • Passive buzzer        │   │  • FEV1 / PEF history  │    │
│   │  • MicroSD logging       │   │  • CSV / PDF export    │    │
│   │  • LiPo + IP5306         │   │  • Open BLE SDK        │    │
│   └──────────────────────────┘   └────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

---

## Two Deliverables

| Deliverable | Tech Stack | Description |
|---|---|---|
| **ESP32-S3 Firmware** | C · ESP-IDF v5.2 · FreeRTOS · Bluedroid | Real-time sensor fusion, DSP pipeline, BLE GATT peripheral, session FSM, SD card logging |
| **Flutter Application** (monorepo) | Dart · Flutter 3.22 · BLoC · Clean Architecture | `befine_ble_sdk` (pub.dev package) + `befine_app` (companion app) |

---

## Key Technical Highlights

### Firmware (ESP32-S3)
- **Dual-core task architecture**: sensor/DSP tasks pinned to APP_CPU, BLE/display to PRO_CPU
- **50 Hz DSP pipeline**: IIR Butterworth low-pass → Bernoulli flow conversion → FEV1/PEF volume integration → peak detection
- **Deterministic FSM**: `IDLE → SHAKING → INSERTING → INHALING → HOLDING → COMPLETE` driven by FreeRTOS event queues
- **GATT peripheral**: custom 16-bit UUIDs (0xBF00–0xBF06), frame-encoded binary protocol
- **Power management**: three modes (ACTIVE 240 MHz / IDLE light-sleep / DEEP SLEEP), 5-min auto-sleep with GPIO+RTC wakeup
- **OTA updates**: dual-partition A/B scheme via ESP-IDF OTA API

### Flutter Application
- **befine_ble_sdk** (open-source pub.dev package): GATT frame parser, auto-reconnect with exponential backoff, reactive Dart streams
- **befine_app**: Clean Architecture (domain/data/presentation), flutter_bloc FSM mirroring device states, fl_chart real-time visualization, Drift local database, go_router navigation, get_it DI
- **Export formats**: CSV (raw flow), PDF (formatted report), JSON (EHR-ready)

---

## GATT Protocol (Firmware ↔ SDK Contract)

| Characteristic | UUID | Direction | Rate | Payload |
|---|---|---|---|---|
| Flow Rate | 0xBF01 | Notify | 50 Hz | `uint16 flow×100` + `uint8 flags` |
| FEV1/PEF Result | 0xBF02 | Indicate | End of session | `float32 fev1` + `float32 pef` + `uint8 score` |
| Environment | 0xBF03 | Notify | 1 Hz | `int16 temp×100` + `uint16 humidity×100` |
| Session Control | 0xBF04 | Write | On-demand | `0x01`=START `0x02`=STOP `0x03`=RESET `0xF0`=CAL |
| Device Info | 0xBF05 | Read | On-demand | fw_major, fw_minor, hw_rev, battery% |
| Error Status | 0xBF06 | Notify | On-event | `uint8 error_code` + `uint32 timestamp_ms` |

---

## Repository Layout

```
PFA2026/BeFine/
├── ESP-IDF/                     # Firmware (C / ESP-IDF)
│   ├── main/                    # Entry point + config header
│   └── components/
│       ├── sensors/             # SDP3x, SHT4x, MPU6050 drivers
│       ├── dsp/                 # IIR filter, flow calc, FEV1 integrator
│       ├── session/             # Session FSM
│       ├── bluetooth/           # GATT server (Bluedroid)
│       ├── display/             # SSD1306 OLED + screen renderers
│       ├── storage/             # SPIFFS + SD card logging
│       └── power_mgmt/          # esp_pm power profiles
│
└── BeFine/                      # Flutter monorepo (Dart)
    ├── packages/
    │   └── befine_ble_sdk/      # pub.dev open-source package
    └── apps/
        └── befine_app/          # Companion application
```

---

## Why This Project?

- Demonstrates **full-stack IoT** competency: hardware-adjacent C firmware through to polished Flutter UI
- Real-world **medical device constraints**: deterministic real-time behavior, data integrity (CRC), low-power operation
- Production-grade software patterns: Clean Architecture, BLoC, repository pattern, dependency injection, unit-testable DSP
- Open-source friendly: MIT license, pub.dev package, well-documented GATT contract
- Strong **GitHub portfolio signal**: firmware + SDK + app in a single monorepo, CI-ready, conventional commits

---

## Tech Stack Summary

| Layer | Technology |
|---|---|
| MCU | ESP32-S3 (240 MHz dual-core, BLE 5.0, 8 MB Flash) |
| RTOS | FreeRTOS (via ESP-IDF v5.2) |
| Firmware language | C (MISRA-inspired, no heap in real-time paths) |
| BLE stack | Bluedroid (ESP-IDF native) |
| Sensors | Sensirion SDP31, SHT41 · InvenSense MPU-6050 |
| App framework | Flutter 3.22 / Dart 3.4 |
| State management | flutter_bloc |
| Architecture | Clean Architecture (domain / data / presentation) |
| Local DB | Drift (SQLite) |
| Navigation | go_router |
| DI | get_it |
| Charts | fl_chart |
| BLE (Flutter) | flutter_blue_plus |
| Testing | Unity (firmware) · flutter_test (app) |
