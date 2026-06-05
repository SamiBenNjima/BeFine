import 'package:flutter/foundation.dart';

@immutable
class DeviceInfo {
  const DeviceInfo({
    required this.fwMajor,
    required this.fwMinor,
    required this.hwRev,
    required this.batteryPct,
  });

  final int fwMajor;
  final int fwMinor;
  final int hwRev;
  final int batteryPct;

  String get firmwareVersion => '$fwMajor.$fwMinor';

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is DeviceInfo &&
          fwMajor == other.fwMajor &&
          fwMinor == other.fwMinor &&
          hwRev == other.hwRev &&
          batteryPct == other.batteryPct;

  @override
  int get hashCode => Object.hash(fwMajor, fwMinor, hwRev, batteryPct);
}
