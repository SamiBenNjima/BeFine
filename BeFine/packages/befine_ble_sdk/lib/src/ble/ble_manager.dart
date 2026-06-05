import 'dart:async';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import '../gatt/befine_gatt_profile.dart';
import '../gatt/frame_parser.dart';
import '../models/flow_measurement.dart';
import '../models/environment_data.dart';
import '../models/session_result.dart';
import '../exceptions/ble_exception.dart';
import 'reconnect_policy.dart';
import 'connection_state.dart';

/// Central BLE orchestrator — singleton via get_it in the app.
/// Exposes reactive streams that survive connection drops via
/// exponential-backoff reconnect.
class BleManager {
  BleManager({required this.policy});

  final ReconnectPolicy policy;

  // ── Private state ─────────────────────────────────────────────────────────
  BluetoothDevice? _device;
  int _reconnectAttempt = 0;
  Timer? _reconnectTimer;
  StreamSubscription<BluetoothConnectionState>? _connStateSub;
  StreamSubscription<List<int>>? _flowSub;
  StreamSubscription<List<int>>? _envSub;
  StreamSubscription<List<int>>? _resultSub;

  BluetoothCharacteristic? _sessionCtrlChar;

  // ── Output stream controllers ─────────────────────────────────────────────
  final _flowCtrl = StreamController<FlowMeasurement>.broadcast();
  final _envCtrl = StreamController<EnvironmentData>.broadcast();
  final _resultCtrl = StreamController<SessionResult>.broadcast();
  final _connCtrl =
      StreamController<BefineConnectionState>.broadcast();

  // ── Public streams ────────────────────────────────────────────────────────
  Stream<FlowMeasurement> get flowStream => _flowCtrl.stream;
  Stream<EnvironmentData> get environmentStream => _envCtrl.stream;
  Stream<SessionResult> get resultStream => _resultCtrl.stream;
  Stream<BefineConnectionState> get connectionStream => _connCtrl.stream;

  // ── Connect ───────────────────────────────────────────────────────────────
  Future<void> connect(BluetoothDevice device) async {
    _device = device;
    _reconnectAttempt = 0;
    await _connectInternal();
  }

  Future<void> _connectInternal() async {
    _connCtrl.add(const Connecting());
    try {
      await _device!.connect(timeout: const Duration(seconds: 10));
      _reconnectAttempt = 0;
      _connCtrl.add(const Connected());
      await _subscribeCharacteristics();

      _connStateSub = _device!.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _connCtrl.add(const Disconnected());
          _teardownSubscriptions();
          _scheduleReconnect();
        }
      });
    } on Exception catch (e) {
      _scheduleReconnect(reason: e.toString());
    }
  }

  void _scheduleReconnect({String? reason}) {
    if (_reconnectAttempt >= policy.maxAttempts) {
      _connCtrl.add(ConnectionFailed(
        reason: reason ?? 'Max reconnect attempts reached',
      ));
      return;
    }
    final delay = policy.delayFor(_reconnectAttempt);
    _connCtrl.add(Reconnecting(attempt: ++_reconnectAttempt));
    _reconnectTimer = Timer(delay, _connectInternal);
  }

  // ── Subscribe to GATT characteristics ─────────────────────────────────────
  Future<void> _subscribeCharacteristics() async {
    final services = await _device!.discoverServices();
    final svc = services.firstWhere(
      (s) => s.serviceUuid == BefineGattProfile.spirometryServiceUuid,
      orElse: () => throw BleException.notSupported(),
    );

    final chars = {for (final c in svc.characteristics) c.characteristicUuid: c};

    // Flow Rate — Notify
    final flowChar = chars[BefineGattProfile.flowRateUuid];
    if (flowChar != null) {
      await flowChar.setNotifyValue(true);
      _flowSub = flowChar.lastValueStream
          .where((b) => b.isNotEmpty)
          .map(FrameParser.parseFlowRate)
          .listen(_flowCtrl.add);
    }

    // Environment — Notify
    final envChar = chars[BefineGattProfile.environmentUuid];
    if (envChar != null) {
      await envChar.setNotifyValue(true);
      _envSub = envChar.lastValueStream
          .where((b) => b.isNotEmpty)
          .map(FrameParser.parseEnvironment)
          .listen(_envCtrl.add);
    }

    // Result — Indicate
    final resultChar = chars[BefineGattProfile.resultUuid];
    if (resultChar != null) {
      await resultChar.setNotifyValue(true);
      _resultSub = resultChar.lastValueStream
          .where((b) => b.isNotEmpty)
          .map(FrameParser.parseResult)
          .listen(_resultCtrl.add);
    }

    // Session Control — Write (keep reference for sendCommand)
    _sessionCtrlChar = chars[BefineGattProfile.sessionCtrlUuid];
  }

  // ── Send command ──────────────────────────────────────────────────────────
  Future<void> sendCommand(SessionCommand cmd) async {
    if (_sessionCtrlChar == null) {
      throw BleException.connectionFailed('Session control characteristic not found');
    }
    await _sessionCtrlChar!.write(cmd.bytes, withoutResponse: false);
  }

  // ── Teardown ──────────────────────────────────────────────────────────────
  void _teardownSubscriptions() {
    _flowSub?.cancel();
    _envSub?.cancel();
    _resultSub?.cancel();
    _connStateSub?.cancel();
    _flowSub = _envSub = _resultSub = _connStateSub = null;
    _sessionCtrlChar = null;
  }

  Future<void> disconnect() async {
    _reconnectTimer?.cancel();
    _teardownSubscriptions();
    await _device?.disconnect();
    _connCtrl.add(const Disconnected());
  }

  Future<void> dispose() async {
    _reconnectTimer?.cancel();
    _teardownSubscriptions();
    await _device?.disconnect();
    await _flowCtrl.close();
    await _envCtrl.close();
    await _resultCtrl.close();
    await _connCtrl.close();
  }
}
