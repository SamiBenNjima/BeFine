import 'dart:typed_data';
import 'package:flutter_test/flutter_test.dart';
import 'package:befine_ble_sdk/befine_ble_sdk.dart';

void main() {
  group('FrameParser.parseFlowRate', () {
    test('decodes 12.50 L/min inhaling frame correctly', () {
      // 1250 = 0x04E2 → LE bytes [0xE2, 0x04], flags = 0x01 (inhaling)
      final m = FrameParser.parseFlowRate([0xE2, 0x04, 0x01]);
      expect(m.flowLpm, closeTo(12.50, 0.01));
      expect(m.isInhaling, isTrue);
      expect(m.isExhaling, isFalse);
    });

    test('decodes zero flow with no flags', () {
      final m = FrameParser.parseFlowRate([0x00, 0x00, 0x00]);
      expect(m.flowLpm, closeTo(0.0, 0.001));
      expect(m.isInhaling, isFalse);
    });

    test('decodes max realistic flow: 600 L/min', () {
      // 60000 = 0xEA60 → LE [0x60, 0xEA]
      final m = FrameParser.parseFlowRate([0x60, 0xEA, 0x05]);
      expect(m.flowLpm, closeTo(600.0, 0.1));
    });

    test('throws ParseException on truncated frame (2 bytes)', () {
      expect(
        () => FrameParser.parseFlowRate([0xE2, 0x04]),
        throwsA(isA<ParseException>()),
      );
    });

    test('throws ParseException on empty frame', () {
      expect(
        () => FrameParser.parseFlowRate([]),
        throwsA(isA<ParseException>()),
      );
    });
  });

  group('FrameParser.parseEnvironment', () {
    test('decodes 23.45 °C and 65.00 %rH', () {
      // temp: 2345 = 0x0929 → LE [0x29, 0x09]
      // hum:  6500 = 0x1964 → LE [0x64, 0x19]
      final e = FrameParser.parseEnvironment([0x29, 0x09, 0x64, 0x19]);
      expect(e.temperatureC, closeTo(23.45, 0.01));
      expect(e.humidityPct, closeTo(65.00, 0.01));
    });

    test('handles negative temperature (-5.00 °C)', () {
      // -500 = 0xFE0C in int16 → LE [0x0C, 0xFE]
      final e = FrameParser.parseEnvironment([0x0C, 0xFE, 0x00, 0x00]);
      expect(e.temperatureC, closeTo(-5.00, 0.01));
    });

    test('throws ParseException on short frame', () {
      expect(
        () => FrameParser.parseEnvironment([0x29, 0x09]),
        throwsA(isA<ParseException>()),
      );
    });
  });

  group('FrameParser.parseResult', () {
    test('decodes FEV1=1.5L PEF=300 score=4 (A)', () {
      // Build 9-byte frame
      final buf = ByteData(9);
      buf.setFloat32(0, 1.5, Endian.little);
      buf.setFloat32(4, 300.0, Endian.little);
      buf.setUint8(8, 4); // score A
      final r = FrameParser.parseResult(buf.buffer.asUint8List());
      expect(r.fev1L, closeTo(1.5, 0.001));
      expect(r.pefLpm, closeTo(300.0, 0.1));
      expect(r.qualityScore, ScoreGrade.a);
    });

    test('throws ParseException on short frame', () {
      expect(
        () => FrameParser.parseResult([0x00, 0x01]),
        throwsA(isA<ParseException>()),
      );
    });
  });

  group('ReconnectPolicy', () {
    test('delay increases with each attempt', () {
      const policy = ReconnectPolicy(
        maxAttempts: 5,
        initialDelay: Duration(milliseconds: 500),
        backoffFactor: 2.0,
        jitterFraction: 0.0, // no jitter for deterministic test
      );
      final d0 = policy.delayFor(0).inMilliseconds;
      final d1 = policy.delayFor(1).inMilliseconds;
      final d2 = policy.delayFor(2).inMilliseconds;
      expect(d1, greaterThan(d0));
      expect(d2, greaterThan(d1));
    });

    test('delay is capped at maxDelay', () {
      const policy = ReconnectPolicy(
        maxAttempts: 5,
        initialDelay: Duration(seconds: 1),
        maxDelay: Duration(seconds: 5),
        backoffFactor: 10.0,
        jitterFraction: 0.0,
      );
      final d10 = policy.delayFor(10);
      expect(d10, lessThanOrEqualTo(const Duration(seconds: 5)));
    });
  });
}
