import 'dart:typed_data';
import '../models/flow_measurement.dart';
import '../models/environment_data.dart';
import '../models/session_result.dart';
import '../exceptions/parse_exception.dart';

/// Stateless, pure parser — zero-copy ByteData view over incoming bytes.
/// Every method is static to make unit testing trivial.
abstract final class FrameParser {
  // ── Flow Rate (0xBF01) — 3 bytes ─────────────────────────────────────────
  // [uint16 LE flow_lpm×100][uint8 flags]
  // flags bit0 = isInhaling, bit1 = isExhaling, bit2 = valid, bit7 = error
  static FlowMeasurement parseFlowRate(List<int> bytes) {
    if (bytes.length < 3) {
      throw ParseException.malformedFrame(
        characteristic: 'FlowRate (0xBF01)',
        expected: 3,
        actual: bytes.length,
      );
    }
    final data = ByteData.sublistView(Uint8List.fromList(bytes));
    final rawFlow = data.getUint16(0, Endian.little);
    final flags = data.getUint8(2);

    return FlowMeasurement(
      flowLpm: rawFlow / 100.0,
      isInhaling: (flags & 0x01) != 0,
      isExhaling: (flags & 0x02) != 0,
      timestamp: DateTime.now(),
    );
  }

  // ── Environment (0xBF03) — 4 bytes ───────────────────────────────────────
  // [int16 LE temp×100][uint16 LE humidity×100]
  static EnvironmentData parseEnvironment(List<int> bytes) {
    if (bytes.length < 4) {
      throw ParseException.malformedFrame(
        characteristic: 'Environment (0xBF03)',
        expected: 4,
        actual: bytes.length,
      );
    }
    final data = ByteData.sublistView(Uint8List.fromList(bytes));
    final rawTemp = data.getInt16(0, Endian.little);
    final rawHum = data.getUint16(2, Endian.little);

    return EnvironmentData(
      temperatureC: rawTemp / 100.0,
      humidityPct: rawHum / 100.0,
      timestamp: DateTime.now(),
    );
  }

  // ── Session Result (0xBF02) — 9 bytes ────────────────────────────────────
  // [float32 LE fev1_L][float32 LE pef_L_s][uint8 score]
  static SessionResult parseResult(List<int> bytes) {
    if (bytes.length < 9) {
      throw ParseException.malformedFrame(
        characteristic: 'Result (0xBF02)',
        expected: 9,
        actual: bytes.length,
      );
    }
    final data = ByteData.sublistView(Uint8List.fromList(bytes));
    final fev1 = data.getFloat32(0, Endian.little);
    final pef = data.getFloat32(4, Endian.little);
    final scoreRaw = data.getUint8(8);

    return SessionResult(
      sessionId: DateTime.now().millisecondsSinceEpoch,
      fev1L: fev1.toDouble(),
      pefLpm: pef.toDouble(),
      qualityScore: ScoreGradeLabel.fromRaw(scoreRaw),
      durationMs: 0,
      timestampUnix: DateTime.now().millisecondsSinceEpoch ~/ 1000,
    );
  }

  // ── Device Info (0xBF05) — 4 bytes ───────────────────────────────────────
  static Map<String, int> parseDeviceInfo(List<int> bytes) {
    if (bytes.length < 4) {
      throw ParseException.malformedFrame(
        characteristic: 'DeviceInfo (0xBF05)',
        expected: 4,
        actual: bytes.length,
      );
    }
    return {
      'fwMajor': bytes[0],
      'fwMinor': bytes[1],
      'hwRev': bytes[2],
      'batteryPct': bytes[3],
    };
  }
}
