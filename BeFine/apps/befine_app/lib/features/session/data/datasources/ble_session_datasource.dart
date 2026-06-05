import 'package:befine_ble_sdk/befine_ble_sdk.dart';

abstract interface class BleSessionDataSource {
  Future<void> sendStart();
  Future<void> sendStop();
  Stream<FlowMeasurement> get flowStream;
  Stream<EnvironmentData> get environmentStream;
  Stream<SessionResult> get resultStream;
}

final class BleSessionDataSourceImpl implements BleSessionDataSource {
  BleSessionDataSourceImpl({required BleManager manager})
      : _manager = manager;

  final BleManager _manager;

  @override
  Future<void> sendStart() => _manager.sendCommand(SessionCommand.start);

  @override
  Future<void> sendStop() => _manager.sendCommand(SessionCommand.stop);

  @override
  Stream<FlowMeasurement> get flowStream => _manager.flowStream;

  @override
  Stream<EnvironmentData> get environmentStream =>
      _manager.environmentStream;

  @override
  Stream<SessionResult> get resultStream => _manager.resultStream;
}
