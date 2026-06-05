import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:equatable/equatable.dart';

class SessionResultEntity extends Equatable {
  const SessionResultEntity({
    required this.sessionId,
    required this.fev1L,
    required this.pefLpm,
    required this.score,
    required this.durationMs,
    required this.recordedAt,
  });

  final int sessionId;
  final double fev1L;
  final double pefLpm;
  final ScoreGrade score;
  final int durationMs;
  final DateTime recordedAt;

  factory SessionResultEntity.fromSdkResult(SessionResult r) =>
      SessionResultEntity(
        sessionId: r.sessionId,
        fev1L: r.fev1L,
        pefLpm: r.pefLpm,
        score: r.qualityScore,
        durationMs: r.durationMs,
        recordedAt:
            DateTime.fromMillisecondsSinceEpoch(r.timestampUnix * 1000),
      );

  @override
  List<Object?> get props =>
      [sessionId, fev1L, pefLpm, score, durationMs, recordedAt];
}
