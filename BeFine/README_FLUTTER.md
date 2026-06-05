# 🌬️ BeFine — Flutter Application

> **BLE Spirometer SDK + Multi-Platform Smart Inhaler Companion**
> Android · iOS · Web · Open-Source · pub.dev

[![Flutter](https://img.shields.io/badge/Flutter-3.22+-02569B?logo=flutter)](https://flutter.dev)
[![Dart](https://img.shields.io/badge/Dart-3.4+-0175C2?logo=dart)](https://dart.dev)
[![pub.dev](https://img.shields.io/badge/pub.dev-befine__ble__sdk-blue)](https://pub.dev)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Android%20%7C%20iOS%20%7C%20Web-lightgrey)]()

---

## 📋 Table of Contents

- [Project Overview](#project-overview)
- [Repository Structure (Monorepo)](#repository-structure-monorepo)
- [BeFine BLE Spirometer SDK](#-befine-ble-spirometer-sdk)
  - [GATT Profile](#gatt-profile)
  - [Frame Parsing](#frame-parsing)
  - [Auto-Reconnect & Dart Streams](#auto-reconnect--dart-streams)
  - [Installation](#installation-sdk)
  - [API Reference](#api-reference)
- [Flutter Application](#-flutter-application)
  - [Architecture (Clean + BLoC)](#architecture-clean--bloc)
  - [Feature Modules](#feature-modules)
  - [Design Patterns Used](#design-patterns-used)
  - [Screens & Navigation](#screens--navigation)
- [Getting Started](#getting-started)
- [Configuration](#configuration)
- [Contributing](#contributing)
- [Roadmap](#roadmap)
- [License](#license)

---

## Project Overview

**BeFine** is a smart inhaler companion ecosystem. The Flutter layer is composed of two publishable artifacts:

| Artifact | Type | Description |
|---|---|---|
| `befine_ble_sdk` | Flutter package (pub.dev) | Open-source BLE abstraction layer: GATT parsing, auto-reconnect, Dart streams |
| `befine_app` | Flutter application | Full companion app: session history, guided procedure, charts, export |

The BLE SDK is intentionally decoupled from the app so third-party developers can integrate BeFine-compatible spirometry hardware into their own health apps.

---

## Repository Structure (Monorepo)

```
befine/
├── packages/
│   └── befine_ble_sdk/              # 📦 pub.dev package (open-source)
│       ├── lib/
│       │   ├── src/
│       │   │   ├── ble/
│       │   │   │   ├── ble_manager.dart          # Central BLE orchestrator
│       │   │   │   ├── ble_scanner.dart           # Device discovery
│       │   │   │   ├── ble_connector.dart         # Connection lifecycle
│       │   │   │   └── reconnect_policy.dart      # Exponential backoff policy
│       │   │   ├── gatt/
│       │   │   │   ├── befine_gatt_profile.dart   # UUID constants
│       │   │   │   ├── frame_parser.dart          # Binary → structured data
│       │   │   │   ├── characteristic_reader.dart
│       │   │   │   └── notification_handler.dart
│       │   │   ├── models/
│       │   │   │   ├── spirometry_sample.dart     # Immutable data class
│       │   │   │   ├── device_info.dart
│       │   │   │   ├── flow_measurement.dart
│       │   │   │   ├── environment_data.dart
│       │   │   │   └── session_data.dart
│       │   │   ├── streams/
│       │   │   │   ├── measurement_stream.dart    # Real-time Dart Stream
│       │   │   │   └── connection_state_stream.dart
│       │   │   └── exceptions/
│       │   │       ├── ble_exception.dart
│       │   │       └── parse_exception.dart
│       │   └── befine_ble_sdk.dart                # Public barrel export
│       ├── test/
│       │   ├── unit/
│       │   │   ├── frame_parser_test.dart
│       │   │   └── reconnect_policy_test.dart
│       │   └── integration/
│       │       └── ble_manager_test.dart
│       ├── example/
│       │   └── lib/main.dart                      # Minimal usage example
│       ├── CHANGELOG.md
│       ├── pubspec.yaml
│       └── README.md
│
├── apps/
│   └── befine_app/                  # 📱 Flutter companion application
│       ├── lib/
│       │   ├── core/
│       │   │   ├── di/
│       │   │   │   └── injection_container.dart   # get_it service locator
│       │   │   ├── router/
│       │   │   │   └── app_router.dart            # go_router configuration
│       │   │   ├── theme/
│       │   │   │   ├── app_theme.dart
│       │   │   │   ├── color_scheme.dart
│       │   │   │   └── text_theme.dart
│       │   │   ├── errors/
│       │   │   │   ├── failure.dart               # Sealed failure hierarchy
│       │   │   │   └── error_handler.dart
│       │   │   └── utils/
│       │   │       ├── extensions.dart
│       │   │       └── validators.dart
│       │   ├── features/
│       │   │   ├── device/
│       │   │   │   ├── data/
│       │   │   │   │   ├── datasources/
│       │   │   │   │   │   └── ble_device_datasource.dart
│       │   │   │   │   ├── models/
│       │   │   │   │   │   └── device_model.dart  # JSON serializable
│       │   │   │   │   └── repositories/
│       │   │   │   │       └── device_repository_impl.dart
│       │   │   │   ├── domain/
│       │   │   │   │   ├── entities/
│       │   │   │   │   │   └── device.dart        # Pure Dart entity
│       │   │   │   │   ├── repositories/
│       │   │   │   │   │   └── device_repository.dart  # abstract
│       │   │   │   │   └── usecases/
│       │   │   │   │       ├── scan_devices.dart
│       │   │   │   │       ├── connect_device.dart
│       │   │   │   │       └── disconnect_device.dart
│       │   │   │   └── presentation/
│       │   │   │       ├── bloc/
│       │   │   │       │   ├── device_bloc.dart
│       │   │   │       │   ├── device_event.dart
│       │   │   │       │   └── device_state.dart
│       │   │   │       ├── pages/
│       │   │   │       │   ├── scan_page.dart
│       │   │   │       │   └── device_detail_page.dart
│       │   │   │       └── widgets/
│       │   │   │           ├── device_tile.dart
│       │   │   │           └── rssi_indicator.dart
│       │   │   ├── session/
│       │   │   │   ├── data/ ...
│       │   │   │   ├── domain/
│       │   │   │   │   ├── entities/
│       │   │   │   │   │   └── session.dart
│       │   │   │   │   └── usecases/
│       │   │   │   │       ├── start_session.dart
│       │   │   │   │       ├── end_session.dart
│       │   │   │   │       └── get_session_history.dart
│       │   │   │   └── presentation/
│       │   │   │       ├── bloc/ ...
│       │   │   │       ├── pages/
│       │   │   │       │   ├── guided_session_page.dart
│       │   │   │       │   └── session_summary_page.dart
│       │   │   │       └── widgets/
│       │   │   │           ├── flow_chart_widget.dart
│       │   │   │           ├── step_indicator.dart
│       │   │   │           └── technique_score_badge.dart
│       │   │   ├── history/
│       │   │   │   └── ... (standard Clean Architecture layers)
│       │   │   ├── settings/
│       │   │   │   └── ...
│       │   │   └── export/
│       │   │       └── ...
│       │   └── main.dart
│       ├── test/
│       ├── integration_test/
│       ├── assets/
│       │   ├── animations/          # Lottie .json files
│       │   ├── icons/
│       │   └── sounds/
│       └── pubspec.yaml
│
├── melos.yaml                        # Monorepo tooling
├── analysis_options.yaml
└── README.md
```

---

## 📦 BeFine BLE Spirometer SDK

> **Package name:** `befine_ble_sdk`
> **Publisher:** `befine.dev`
> **License:** MIT

A Flutter-first, open-source BLE abstraction layer for spirometry and respiratory measurement devices. Compatible with any device implementing the **BeFine GATT Profile**.

### GATT Profile

All BeFine-compatible hardware exposes the following GATT structure:

```
Service: BeFine Spirometry Service
UUID: 0xBF00  (custom 128-bit: BE-FI-NE-00-...)

├── Characteristic: Flow Rate (Notify)
│   UUID: 0xBF01
│   Format: [uint16 flow_lpm_x100][uint8 flags]
│   Notify interval: 20 ms (50 Hz)
│
├── Characteristic: FEV1 / PEF Result (Indicate)
│   UUID: 0xBF02
│   Format: [float32 fev1_L][float32 pef_L_s][uint8 quality]
│
├── Characteristic: Environment (Notify)
│   UUID: 0xBF03
│   Format: [int16 temp_x100][uint16 humidity_x100]
│   Notify interval: 1 s
│
├── Characteristic: Session Control (Write)
│   UUID: 0xBF04
│   Commands: 0x01=START 0x02=STOP 0x03=RESET 0xF0=CALIBRATE
│
├── Characteristic: Device Info (Read)
│   UUID: 0xBF05
│   Format: [uint8 fw_major][uint8 fw_minor][uint8 hw_rev][uint8 battery_pct]
│
└── Characteristic: Error Status (Notify)
    UUID: 0xBF06
    Format: [uint8 error_code][uint32 timestamp_ms]
```

#### Error Codes

| Code | Meaning |
|------|---------|
| `0x00` | No error |
| `0x01` | Sensor initialization failure |
| `0x02` | Flow sensor timeout |
| `0x03` | Memory full |
| `0x10` | Battery critical |
| `0xFF` | Unknown / fatal |

---

### Frame Parsing

The SDK's `FrameParser` converts raw `List<int>` GATT notification bytes into typed Dart objects using a zero-copy `ByteData` view:

```dart
// lib/src/gatt/frame_parser.dart

import 'dart:typed_data';
import '../models/flow_measurement.dart';
import '../models/environment_data.dart';
import '../exceptions/parse_exception.dart';

/// Stateless, pure parser — easily unit-testable.
abstract final class FrameParser {

  /// Parses a Flow Rate characteristic frame (0xBF01).
  /// Frame: [uint16 LE flow_lpm_x100][uint8 flags]
  static FlowMeasurement parseFlowRate(List<int> bytes) {
    if (bytes.length < 3) {
      throw ParseException.malformedFrame(
        characteristic: 'FlowRate',
        expected: 3,
        actual: bytes.length,
      );
    }

    final data = ByteData.sublistView(Uint8List.fromList(bytes));
    final rawFlow  = data.getUint16(0, Endian.little);   // lpm × 100
    final flags    = data.getUint8(2);

    return FlowMeasurement(
      flowLpm:     rawFlow / 100.0,
      isInhaling:  (flags & 0x01) != 0,
      isExhaling:  (flags & 0x02) != 0,
      timestamp:   DateTime.now(),
    );
  }

  /// Parses an Environment characteristic frame (0xBF03).
  static EnvironmentData parseEnvironment(List<int> bytes) {
    if (bytes.length < 4) {
      throw ParseException.malformedFrame(
        characteristic: 'Environment',
        expected: 4,
        actual: bytes.length,
      );
    }

    final data     = ByteData.sublistView(Uint8List.fromList(bytes));
    final rawTemp  = data.getInt16(0, Endian.little);    // °C × 100
    final rawHum   = data.getUint16(2, Endian.little);   // %rH × 100

    return EnvironmentData(
      temperatureC: rawTemp / 100.0,
      humidityPct:  rawHum  / 100.0,
      timestamp:    DateTime.now(),
    );
  }
}
```

**Unit test example:**

```dart
// test/unit/frame_parser_test.dart

void main() {
  group('FrameParser.parseFlowRate', () {
    test('correctly decodes 12.50 L/min inhaling frame', () {
      // 1250 = 0x04E2 in LE → [0xE2, 0x04], flags=0x01 (inhaling)
      final sample = FrameParser.parseFlowRate([0xE2, 0x04, 0x01]);
      expect(sample.flowLpm,    closeTo(12.50, 0.001));
      expect(sample.isInhaling, isTrue);
      expect(sample.isExhaling, isFalse);
    });

    test('throws ParseException on truncated frame', () {
      expect(
        () => FrameParser.parseFlowRate([0x00]),
        throwsA(isA<ParseException>()),
      );
    });
  });
}
```

---

### Auto-Reconnect & Dart Streams

The SDK exposes **reactive streams** that stay alive across connection drops via an exponential-backoff reconnect policy.

```dart
// lib/src/ble/reconnect_policy.dart

/// Immutable configuration for reconnect behaviour.
class ReconnectPolicy {
  const ReconnectPolicy({
    this.maxAttempts      = 5,
    this.initialDelay     = const Duration(milliseconds: 500),
    this.maxDelay         = const Duration(seconds: 30),
    this.backoffFactor    = 2.0,
    this.jitterFraction   = 0.1,
  });

  final int      maxAttempts;
  final Duration initialDelay;
  final Duration maxDelay;
  final double   backoffFactor;
  final double   jitterFraction;

  Duration delayFor(int attempt) {
    final base = initialDelay * math.pow(backoffFactor, attempt);
    final capped = base > maxDelay ? maxDelay : base;
    final jitter = capped * jitterFraction * (math.Random().nextDouble() - 0.5);
    return capped + jitter;
  }
}
```

```dart
// lib/src/ble/ble_manager.dart

/// Central orchestrator — singleton via get_it.
class BleManager {
  BleManager({
    required this.policy,
    FlutterBluePlus? bleClient,        // injectable for testing
  }) : _ble = bleClient ?? FlutterBluePlus();

  final ReconnectPolicy policy;
  final FlutterBluePlus _ble;

  // ── Public Streams ──────────────────────────────────────────────────────

  /// Real-time flow measurements at ~50 Hz. Survives reconnects.
  Stream<FlowMeasurement> get flowStream => _flowController.stream;

  /// Environmental readings at ~1 Hz.
  Stream<EnvironmentData> get environmentStream => _envController.stream;

  /// Device connection state updates.
  Stream<ConnectionState> get connectionStream => _connController.stream;

  // ── Private ─────────────────────────────────────────────────────────────

  final _flowController = StreamController<FlowMeasurement>.broadcast();
  final _envController  = StreamController<EnvironmentData>.broadcast();
  final _connController = StreamController<ConnectionState>.broadcast();

  BluetoothDevice? _device;
  int _reconnectAttempt = 0;
  Timer? _reconnectTimer;

  /// Initiate connection and begin streaming.
  Future<void> connect(BluetoothDevice device) async {
    _device = device;
    _reconnectAttempt = 0;
    await _connectInternal();
  }

  Future<void> _connectInternal() async {
    try {
      await _device!.connect(timeout: const Duration(seconds: 10));
      _reconnectAttempt = 0;
      _connController.add(ConnectionState.connected);
      await _subscribeCharacteristics();

      // Listen for disconnection
      _device!.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _connController.add(ConnectionState.disconnected);
          _scheduleReconnect();
        }
      });
    } on TimeoutException {
      _scheduleReconnect();
    }
  }

  void _scheduleReconnect() {
    if (_reconnectAttempt >= policy.maxAttempts) {
      _connController.add(ConnectionState.failed);
      return;
    }
    final delay = policy.delayFor(_reconnectAttempt++);
    _connController.add(ConnectionState.reconnecting(attempt: _reconnectAttempt));
    _reconnectTimer = Timer(delay, _connectInternal);
  }

  Future<void> _subscribeCharacteristics() async {
    final services = await _device!.discoverServices();
    final spirometryService = services.firstWhere(
      (s) => s.uuid == BefineGattProfile.spirometryServiceUuid,
    );

    // Subscribe to Flow Rate notifications
    final flowChar = spirometryService.characteristics.firstWhere(
      (c) => c.uuid == BefineGattProfile.flowRateUuid,
    );
    await flowChar.setNotifyValue(true);
    flowChar.lastValueStream
        .map(FrameParser.parseFlowRate)
        .listen(_flowController.add);

    // Subscribe to Environment notifications
    final envChar = spirometryService.characteristics.firstWhere(
      (c) => c.uuid == BefineGattProfile.environmentUuid,
    );
    await envChar.setNotifyValue(true);
    envChar.lastValueStream
        .map(FrameParser.parseEnvironment)
        .listen(_envController.add);
  }

  Future<void> dispose() async {
    _reconnectTimer?.cancel();
    await _flowController.close();
    await _envController.close();
    await _connController.close();
    await _device?.disconnect();
  }
}
```

---

### Installation (SDK)

Add to your `pubspec.yaml`:

```yaml
dependencies:
  befine_ble_sdk: ^1.0.0
```

#### Android permissions (`AndroidManifest.xml`)

```xml
<uses-permission android:name="android.permission.BLUETOOTH_SCAN"
    android:usesPermissionFlags="neverForLocation" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"
    android:maxSdkVersion="30" />
```

#### iOS permissions (`Info.plist`)

```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>BeFine uses Bluetooth to communicate with your smart inhaler.</string>
<key>NSBluetoothPeripheralUsageDescription</key>
<string>BeFine uses Bluetooth to communicate with your smart inhaler.</string>
```

---

### API Reference

```dart
import 'package:befine_ble_sdk/befine_ble_sdk.dart';

// 1. Create the manager (use get_it in production)
final manager = BleManager(
  policy: const ReconnectPolicy(maxAttempts: 5),
);

// 2. Scan for nearby BeFine devices
final scanner = BleScanner();
final devices = await scanner.scan(timeout: const Duration(seconds: 5));

// 3. Connect
await manager.connect(devices.first);

// 4. Listen to real-time flow measurements
manager.flowStream.listen((FlowMeasurement m) {
  print('Flow: ${m.flowLpm} L/min  inhaling=${m.isInhaling}');
});

// 5. Listen to connection state changes
manager.connectionStream.listen((ConnectionState s) {
  if (s is Reconnecting) print('Reconnecting... attempt ${s.attempt}');
  if (s == ConnectionState.failed) showError();
});

// 6. Send session control command
await manager.sendCommand(SessionCommand.start);

// 7. Clean up
await manager.dispose();
```

---

## 📱 Flutter Application

### Architecture: Clean + BLoC

The app strictly follows **Clean Architecture** with **flutter_bloc** for state management, **get_it** for dependency injection, and **go_router** for declarative navigation.

```
Presentation Layer  ──►  BLoC  ──►  Use Cases  ──►  Repository (abstract)
                                                           │
                                              ┌────────────┴────────────┐
                                        Local DB (drift)        BLE SDK / Remote
```

**Dependency inversion** is enforced at every layer boundary. No `import` from `data/` ever reaches `domain/`; only the DI container wires them.

---

### Feature Modules

#### 1. `device` — BLE Scanning & Connection

| Component | Description |
|---|---|
| `ScanPage` | Animated list of discovered BLE devices with signal strength |
| `DeviceBloc` | Manages scan lifecycle, connection state, reconnect status |
| `ConnectDevice` (use case) | Calls `BleManager.connect()`, maps failures to `Failure` sealed class |
| `DeviceRepositoryImpl` | Wraps SDK's `BleManager`, maps models to domain entities |

#### 2. `session` — Guided Inhaler Procedure

| Component | Description |
|---|---|
| `GuidedSessionPage` | Step-by-step animated procedure screen (shake → insert → inhale → hold) |
| `SessionBloc` | Finite state machine: `Idle → Shaking → Inserting → Inhaling → Holding → Complete` |
| `FlowChartWidget` | Real-time `fl_chart` line chart of flow vs. time during inhalation |
| `TechniqueScoreBadge` | Color-coded quality score (A/B/C/F) based on PEF, FEV1, hold duration |
| `StartSession` (use case) | Sends `SessionCommand.start` to device, persists metadata |

#### 3. `history` — Session History & Trends

| Component | Description |
|---|---|
| `HistoryPage` | Paginated list of past sessions with sparklines |
| `TrendChart` | Weekly/monthly PEF and FEV1 trend using `fl_chart` |
| `GetSessionHistory` (use case) | Queries local Drift database, supports date-range filtering |

#### 4. `export` — Data Export

| Format | Details |
|---|---|
| CSV | Raw flow measurements per session |
| PDF | Formatted session report with charts (using `pdf` package) |
| JSON | Full structured session data for EHR integration |

---

### Design Patterns Used

#### Repository Pattern

```dart
// domain/repositories/session_repository.dart
abstract interface class SessionRepository {
  Future<Either<Failure, Session>> startSession(DeviceId deviceId);
  Future<Either<Failure, SessionResult>> endSession(SessionId id);
  Stream<Either<Failure, FlowMeasurement>> flowMeasurements(SessionId id);
  Future<Either<Failure, List<Session>>> getHistory({DateRange? range});
}
```

```dart
// data/repositories/session_repository_impl.dart
final class SessionRepositoryImpl implements SessionRepository {
  const SessionRepositoryImpl({
    required this.bleDataSource,
    required this.localDataSource,
  });

  final BleDeviceDataSource bleDataSource;
  final LocalSessionDataSource localDataSource;

  @override
  Future<Either<Failure, Session>> startSession(DeviceId deviceId) async {
    try {
      await bleDataSource.sendCommand(SessionCommand.start);
      final session = await localDataSource.createSession(deviceId);
      return Right(session.toDomain());
    } on BleException catch (e) {
      return Left(BleFailure(message: e.message));
    } on StorageException catch (e) {
      return Left(StorageFailure(message: e.message));
    }
  }
}
```

#### BLoC Pattern (FSM-driven session)

```dart
// presentation/bloc/session_bloc.dart

sealed class SessionState {}
final class SessionIdle       extends SessionState {}
final class SessionShaking    extends SessionState {}
final class SessionInserting  extends SessionState {}
final class SessionInhaling   extends SessionState {
  const SessionInhaling({required this.currentFlow});
  final FlowMeasurement currentFlow;
}
final class SessionHolding    extends SessionState { /* countdown */ }
final class SessionComplete   extends SessionState {
  const SessionComplete({required this.result});
  final SessionResult result;
}
final class SessionError      extends SessionState {
  const SessionError({required this.failure});
  final Failure failure;
}

class SessionBloc extends Bloc<SessionEvent, SessionState> {
  SessionBloc({
    required this.startSession,
    required this.endSession,
  }) : super(SessionIdle()) {

    on<SessionStartRequested>(_onStart);
    on<FlowMeasurementReceived>(_onFlow);
    on<SessionStopRequested>(_onStop);
  }

  final StartSession startSession;
  final EndSession   endSession;

  StreamSubscription<FlowMeasurement>? _flowSub;

  Future<void> _onStart(
    SessionStartRequested event,
    Emitter<SessionState> emit,
  ) async {
    final result = await startSession(event.deviceId);
    result.fold(
      (failure) => emit(SessionError(failure: failure)),
      (session) {
        emit(SessionShaking());
        _flowSub = session.flowStream.listen(
          (m) => add(FlowMeasurementReceived(measurement: m)),
        );
      },
    );
  }

  void _onFlow(
    FlowMeasurementReceived event,
    Emitter<SessionState> emit,
  ) {
    // Transition through FSM states based on measurement data
    final m = event.measurement;
    if (state is SessionShaking && m.flowLpm > 0.5) {
      emit(SessionInserting());
    } else if (state is SessionInserting && m.isInhaling) {
      emit(SessionInhaling(currentFlow: m));
    } else if (state is SessionInhaling) {
      emit(SessionInhaling(currentFlow: m));  // update chart
      if (m.flowLpm < 0.1) emit(SessionHolding());
    }
  }
}
```

#### Use Case Pattern (Single Responsibility)

```dart
// domain/usecases/start_session.dart

/// Single-responsibility use case following Uncle Bob's Clean Architecture.
final class StartSession {
  const StartSession(this._repository);
  final SessionRepository _repository;

  Future<Either<Failure, Session>> call(DeviceId deviceId) =>
      _repository.startSession(deviceId);
}
```

#### Observer Pattern (via Streams)

All BLE measurements propagate through Dart's native `Stream` infrastructure — BLoC `listen()`s, UI `StreamBuilder`s, and local DB writers all consume the same broadcast stream without tight coupling.

#### Singleton via Service Locator

```dart
// core/di/injection_container.dart

final sl = GetIt.instance;

Future<void> configureDependencies() async {
  // External
  sl.registerLazySingleton(() => BleManager(policy: const ReconnectPolicy()));
  sl.registerLazySingleton(() => BleScanner());

  // Data sources
  sl.registerLazySingleton<BleDeviceDataSource>(
    () => BleDeviceDataSourceImpl(bleManager: sl()),
  );
  sl.registerLazySingleton<LocalSessionDataSource>(
    () => DriftSessionDataSource(database: AppDatabase()),
  );

  // Repositories
  sl.registerLazySingleton<SessionRepository>(
    () => SessionRepositoryImpl(
      bleDataSource:   sl(),
      localDataSource: sl(),
    ),
  );

  // Use cases
  sl.registerFactory(() => StartSession(sl()));
  sl.registerFactory(() => EndSession(sl()));
  sl.registerFactory(() => GetSessionHistory(sl()));

  // BLoCs
  sl.registerFactory(() => SessionBloc(
    startSession: sl(),
    endSession:   sl(),
  ));
  sl.registerFactory(() => DeviceBloc(
    scanDevices:    sl(),
    connectDevice:  sl(),
  ));
}
```

---

### Screens & Navigation

```
/                  → HomeScreen (session summary + quick-start)
/scan              → ScanPage (discover devices)
/connect/:id       → DeviceDetailPage
/session           → GuidedSessionPage (real-time procedure)
/session/summary   → SessionSummaryPage (FEV1, PEF, score, chart)
/history           → HistoryPage
/history/:id       → SessionDetailPage
/settings          → SettingsPage (patient profile, units, reminders)
/export/:id        → ExportPage (CSV / PDF / JSON)
```

Navigation is handled by **go_router** with nested shell routes for bottom navigation:

```dart
// core/router/app_router.dart
final router = GoRouter(
  initialLocation: '/',
  routes: [
    ShellRoute(
      builder: (_, __, child) => ScaffoldWithNavBar(child: child),
      routes: [
        GoRoute(path: '/', builder: (_, __) => const HomeScreen()),
        GoRoute(path: '/history', builder: (_, __) => const HistoryPage()),
        GoRoute(path: '/settings', builder: (_, __) => const SettingsPage()),
      ],
    ),
    GoRoute(path: '/scan', builder: (_, __) => const ScanPage()),
    GoRoute(path: '/session', builder: (_, __) => const GuidedSessionPage()),
    GoRoute(
      path: '/session/summary',
      builder: (_, state) => SessionSummaryPage(
        result: state.extra as SessionResult,
      ),
    ),
  ],
);
```

---

## Getting Started

### Prerequisites

| Tool | Minimum Version |
|------|----------------|
| Flutter | 3.22.0 |
| Dart | 3.4.0 |
| Melos | 4.x |
| Android SDK | API 26+ |
| Xcode | 15+ (iOS 13+) |

### Clone & Bootstrap

```bash
git clone https://github.com/befine-dev/befine.git
cd befine

# Install melos (monorepo tooling)
dart pub global activate melos

# Bootstrap all packages
melos bootstrap

# Run the app
cd apps/befine_app
flutter run
```

### Run Tests

```bash
# All packages
melos run test

# SDK only
cd packages/befine_ble_sdk
flutter test

# App unit tests
cd apps/befine_app
flutter test

# Integration tests (requires connected device or emulator)
flutter test integration_test/
```

---

## Configuration

```dart
// apps/befine_app/lib/core/config/app_config.dart

abstract final class AppConfig {
  static const bleServiceUuid      = '0000BF00-0000-1000-8000-00805F9B34FB';
  static const scanTimeout         = Duration(seconds: 10);
  static const reconnectMaxAttempts = 5;
  static const flowChartWindowSec  = 6.0;   // seconds visible in chart
  static const holdDurationTarget  = 10;    // seconds to hold breath
  static const minPeakFlowLpm     = 30.0;   // L/min minimum for valid session
}
```

---

## Contributing

1. Fork the repository and create a feature branch: `git checkout -b feat/your-feature`
2. Commit using [Conventional Commits](https://www.conventionalcommits.org): `feat(sdk): add FEV6 parsing`
3. Ensure `flutter analyze` and `flutter test` pass with zero warnings
4. Open a PR — the CI (GitHub Actions) will run tests on Android, iOS, and Web
5. A maintainer will review and merge

### Code Style

- Follow [Effective Dart](https://dart.dev/effective-dart)
- All public APIs must have `///` documentation comments
- Models must be **immutable** (`final` fields, `copyWith`, `==` and `hashCode`)
- Use `sealed class` for states and failures — exhaustive pattern matching

---

## Roadmap

- [x] v1.0 — BLE SDK core (scan, connect, parse, streams, reconnect)
- [x] v1.0 — Flutter app MVP (guided session, history, export)
- [ ] v1.1 — HealthKit / Google Health Connect integration
- [ ] v1.2 — AI-powered technique coaching (on-device ML with TFLite)
- [ ] v1.3 — Multi-patient / caregiver mode
- [ ] v2.0 — Web Bluetooth support (Chrome desktop)
- [ ] v2.0 — BeFine Cloud sync (optional, HIPAA-ready)

---

## License

```
MIT License

Copyright (c) 2025 BeFine Dev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

See [LICENSE](LICENSE) for full text.

---

*Made with ❤️ for better respiratory health.*
