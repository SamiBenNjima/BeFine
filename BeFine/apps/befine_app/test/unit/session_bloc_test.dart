import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:bloc_test/bloc_test.dart';
import 'package:dartz/dartz.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:mocktail/mocktail.dart';

import 'package:befine_app/core/errors/failures.dart';
import 'package:befine_app/features/session/domain/entities/session_result_entity.dart';
import 'package:befine_app/features/session/domain/usecases/start_session.dart';
import 'package:befine_app/features/session/domain/usecases/end_session.dart';
import 'package:befine_app/features/session/presentation/bloc/session_bloc.dart';

// ── Mocks ──────────────────────────────────────────────────────────────────
class MockStartSession extends Mock implements StartSession {}
class MockEndSession extends Mock implements EndSession {}

SessionResultEntity _fakeResult() => SessionResultEntity(
      sessionId: 1,
      fev1L: 1.5,
      pefLpm: 300,
      score: ScoreGrade.a,
      durationMs: 12000,
      recordedAt: DateTime(2025),
    );

void main() {
  late MockStartSession mockStart;
  late MockEndSession mockEnd;

  setUp(() {
    mockStart = MockStartSession();
    mockEnd = MockEndSession();
  });

  group('SessionBloc — FSM transitions', () {
    blocTest<SessionBloc, SessionState>(
      'emits [SessionShaking] when start succeeds',
      build: () {
        when(() => mockStart()).thenAnswer((_) async => const Right(null));
        return SessionBloc(startSession: mockStart, endSession: mockEnd);
      },
      act: (b) => b.add(const SessionStartRequested()),
      expect: () => [isA<SessionShaking>()],
    );

    blocTest<SessionBloc, SessionState>(
      'emits [SessionError] when start fails',
      build: () {
        when(() => mockStart()).thenAnswer((_) async =>
            const Left(BleFailure(message: 'Not connected')));
        return SessionBloc(startSession: mockStart, endSession: mockEnd);
      },
      act: (b) => b.add(const SessionStartRequested()),
      expect: () => [isA<SessionError>()],
    );

    blocTest<SessionBloc, SessionState>(
      'transitions Shaking → Inserting on flow onset',
      build: () {
        when(() => mockStart()).thenAnswer((_) async => const Right(null));
        return SessionBloc(startSession: mockStart, endSession: mockEnd);
      },
      act: (b) async {
        b.add(const SessionStartRequested());
        await Future.delayed(Duration.zero);
        b.pushFlowMeasurement(FlowMeasurement(
          flowLpm: 1.0,
          isInhaling: false,
          isExhaling: false,
          timestamp: DateTime.now(),
        ));
      },
      expect: () => [isA<SessionShaking>(), isA<SessionInserting>()],
    );

    blocTest<SessionBloc, SessionState>(
      'emits [SessionIdle] on reset',
      build: () =>
          SessionBloc(startSession: mockStart, endSession: mockEnd),
      seed: () => const SessionShaking(),
      act: (b) => b.add(const SessionReset()),
      expect: () => [isA<SessionIdle>()],
    );

    blocTest<SessionBloc, SessionState>(
      'emits [SessionComplete] on stop with valid result',
      build: () {
        when(() => mockEnd())
            .thenAnswer((_) async => Right(_fakeResult()));
        return SessionBloc(startSession: mockStart, endSession: mockEnd);
      },
      seed: () => const SessionHolding(),
      act: (b) => b.add(const SessionStopRequested()),
      expect: () => [isA<SessionComplete>()],
    );
  });
}
