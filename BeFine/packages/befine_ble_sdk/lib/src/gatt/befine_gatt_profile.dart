import 'package:flutter_blue_plus/flutter_blue_plus.dart';

/// BeFine GATT Profile — UUID constants.
/// MUST stay in sync with gatt_profile.h in the ESP32-S3 firmware.
abstract final class BefineGattProfile {
  static final Guid spirometryServiceUuid =
      Guid('0000BF00-0000-1000-8000-00805F9B34FB');

  /// Flow Rate — Notify — 3 bytes — [uint16 flow×100 LE][uint8 flags]
  static final Guid flowRateUuid =
      Guid('0000BF01-0000-1000-8000-00805F9B34FB');

  /// FEV1/PEF Result — Indicate — 9 bytes
  static final Guid resultUuid =
      Guid('0000BF02-0000-1000-8000-00805F9B34FB');

  /// Environment — Notify — 4 bytes — [int16 temp×100][uint16 hum×100]
  static final Guid environmentUuid =
      Guid('0000BF03-0000-1000-8000-00805F9B34FB');

  /// Session Control — Write — 1 byte
  static final Guid sessionCtrlUuid =
      Guid('0000BF04-0000-1000-8000-00805F9B34FB');

  /// Device Info — Read — 4 bytes
  static final Guid deviceInfoUuid =
      Guid('0000BF05-0000-1000-8000-00805F9B34FB');

  /// Error Status — Notify — 5 bytes
  static final Guid errorStatusUuid =
      Guid('0000BF06-0000-1000-8000-00805F9B34FB');
}

/// Session control commands (written to 0xBF04).
enum SessionCommand {
  start(0x01),
  stop(0x02),
  reset(0x03),
  calibrate(0xF0);

  const SessionCommand(this.byte);
  final int byte;

  List<int> get bytes => [byte];
}
