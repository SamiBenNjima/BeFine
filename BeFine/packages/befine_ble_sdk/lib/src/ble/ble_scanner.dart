import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import '../gatt/befine_gatt_profile.dart';
import '../exceptions/ble_exception.dart';

class BleScanner {
  /// Scans for BeFine devices (filtered by service UUID 0xBF00).
  Future<List<BluetoothDevice>> scan({
    Duration timeout = const Duration(seconds: 10),
  }) async {
    if (!await FlutterBluePlus.isAvailable) {
      throw BleException.notSupported();
    }

    final found = <BluetoothDevice>[];

    await FlutterBluePlus.startScan(
      withServices: [BefineGattProfile.spirometryServiceUuid],
      timeout: timeout,
    );

    await for (final result in FlutterBluePlus.scanResults) {
      for (final r in result) {
        if (!found.any((d) => d.remoteId == r.device.remoteId)) {
          found.add(r.device);
        }
      }
    }

    await FlutterBluePlus.stopScan();
    return found;
  }
}
