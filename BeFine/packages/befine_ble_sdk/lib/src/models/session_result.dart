import 'package:flutter/foundation.dart';

enum ScoreGrade { a, b, c, f }

extension ScoreGradeLabel on ScoreGrade {
  String get label {
    switch (this) {
      case ScoreGrade.a: return 'A';
      case ScoreGrade.b: return 'B';
      case ScoreGrade.c: return 'C';
      case ScoreGrade.f: return 'F';
    }
  }

  static ScoreGrade fromRaw(int raw) {
    switch (raw) {
      case 4: return ScoreGrade.a;
      case 3:
      case 2: return ScoreGrade.b;
      case 1: return ScoreGrade.c;
      default: return ScoreGrade.f;
    }
  }
}

@immutable
class SessionResult {
  const SessionResult({
    required this.sessionId,
    required this.fev1L,
    required this.pefLpm,
    required this.qualityScore,
    required this.durationMs,
    required this.timestampUnix,
  });

  final int sessionId;
  final double fev1L;
  final double pefLpm;
  final ScoreGrade qualityScore;
  final int durationMs;
  final int timestampUnix;

  SessionResult copyWith({
    int? sessionId,
    double? fev1L,
    double? pefLpm,
    ScoreGrade? qualityScore,
    int? durationMs,
    int? timestampUnix,
  }) =>
      SessionResult(
        sessionId: sessionId ?? this.sessionId,
        fev1L: fev1L ?? this.fev1L,
        pefLpm: pefLpm ?? this.pefLpm,
        qualityScore: qualityScore ?? this.qualityScore,
        durationMs: durationMs ?? this.durationMs,
        timestampUnix: timestampUnix ?? this.timestampUnix,
      );

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is SessionResult && sessionId == other.sessionId;

  @override
  int get hashCode => sessionId.hashCode;
}
