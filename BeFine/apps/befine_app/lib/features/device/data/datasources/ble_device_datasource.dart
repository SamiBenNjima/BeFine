import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import '../../../device/domain/entities/device.dart';

abstract interface class BleDeviceDataSource {
  Future<List<Device>> scan();
  Future<Device> connect(String deviceId);
  Future<void> disconnect(String deviceId);
}

final class BleDeviceDataSourceImpl implements BleDeviceDataSource {
  BleDeviceDataSourceImpl({
    required BleScanner scanner,
    required BleManager manager,
  })  : _scanner = scanner,
        _manager = manager;

  final BleScanner _scanner;
  final BleManager _manager;

  @override
  Future<List<Device>> scan() async {
    final raw = await _scanner.scan();
    return raw
        .map((d) => Device(
              id: d.remoteId.str,
              name: d.platformName.isEmpty ? 'BeFine' : d.platformName,
              rssi: -70, // RSSI requires a scan result object; simplified here
            ))
        .toList();
  }

  @override
  Future<Device> connect(String deviceId) async {
    final raw = await _scanner.scan(timeout: const Duration(seconds: 5));
    final target = raw.firstWhere(
      (d) => d.remoteId.str == deviceId,
      orElse: () => throw BleException.connectionFailed('Device not found'),
    );
    await _manager.connect(target);
    return Device(
      id: target.remoteId.str,
      name: target.platformName.isEmpty ? 'BeFine' : target.platformName,
      rssi: -70,
    );
  }

  @override
  Future<void> disconnect(String deviceId) => _manager.disconnect();
}
