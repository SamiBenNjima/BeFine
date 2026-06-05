import 'dart:async';
import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:equatable/equatable.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import '../../../../core/config/app_config.dart';
import '../../../../core/errors/failures.dart';
import '../../domain/entities/session_result_entity.dart';
import '../../domain/usecases/start_session.dart';
import '../../domain/usecases/end_session.dart';

// ── Events ─────────────────────────────────────────────────────────────────
sealed class SessionEvent extends Equatable {
  const SessionEvent();
  @override
  List<Object?> get props => [];
}

final class SessionStartRequested extends SessionEvent {
  const SessionStartRequested();
}

final class SessionStopRequested extends SessionEvent {
  const SessionStopRequested();
}

final class SessionReset extends SessionEvent {
  const SessionReset();
}

final class _FlowReceived extends SessionEvent {
  const _FlowReceived(this.measurement);
  final FlowMeasurement measurement;
  @override
  List<Object?> get props => [measurement];
}

final class _ResultReceived extends SessionEvent {
  const _ResultReceived(this.result);
  final SessionResultEntity result;
  @override
  List<Object?> get props => [result];
}

// ── States ──────────────────────────────────────────────────────────────────
sealed class SessionState extends Equatable {
  const SessionState();
  @override
  List<Object?> get props => [];
}

final class SessionIdle extends SessionState {
  const SessionIdle();
}

final class SessionShaking extends SessionState {
  const SessionShaking();
}

final class SessionInserting extends SessionState {
  const SessionInserting();
}

final class SessionInhaling extends SessionState {
  const SessionInhaling({required this.currentFlow});
  final FlowMeasurement currentFlow;
  @override
  List<Object?> get props => [currentFlow];
}

final class SessionHolding extends SessionState {
  const SessionHolding({this.remainingSec = 10});
  final int remainingSec;
  @override
  List<Object?> get props => [remainingSec];
}

final class SessionComplete extends SessionState {
  const SessionComplete({required this.result});
  final SessionResultEntity result;
  @override
  List<Object?> get props => [result];
}

final class SessionError extends SessionState {
  const SessionError({required this.failure});
  final Failure failure;
  @override
  List<Object?> get props => [failure];
}

// ── BLoC ────────────────────────────────────────────────────────────────────
class SessionBloc extends Bloc<SessionEvent, SessionState> {
  SessionBloc({
    required StartSession startSession,
    required EndSession endSession,
  })  : _start = startSession,
        _end = endSession,
        super(const SessionIdle()) {
    on<SessionStartRequested>(_onStart);
    on<SessionStopRequested>(_onStop);
    on<SessionReset>(_onReset);
    on<_FlowReceived>(_onFlow);
    on<_ResultReceived>(_onResult);
  }

  final StartSession _start;
  final EndSession _end;
  StreamSubscription<FlowMeasurement>? _flowSub;

  Future<void> _onStart(
    SessionStartRequested event,
    Emitter<SessionState> emit,
  ) async {
    final result = await _start();
    result.fold(
      (failure) => emit(SessionError(failure: failure)),
      (_) {
        emit(const SessionShaking());
        // Subscribe to flow stream from repository via BLoC add
        // (The bloc receives flow through _FlowReceived events)
      },
    );
  }

  void _onFlow(_FlowReceived event, Emitter<SessionState> emit) {
    final m = event.measurement;
    switch (state) {
      case SessionShaking():
        if (m.flowLpm > AppConfig.flowOnsetLpm) {
          emit(const SessionInserting());
        }
      case SessionInserting():
        if (m.isInhaling) {
          emit(SessionInhaling(currentFlow: m));
        }
      case SessionInhaling():
        if (m.flowLpm < AppConfig.flowEndLpm) {
          emit(const SessionHolding());
        } else {
          emit(SessionInhaling(currentFlow: m));
        }
      case SessionHolding():
        if (m.flowLpm > AppConfig.flowOnsetLpm) {
          emit(SessionInhaling(currentFlow: m)); // Resume inhaling
        }
      case _:
        break;
    }
  }

  Future<void> _onStop(
    SessionStopRequested event,
    Emitter<SessionState> emit,
  ) async {
    final result = await _end();
    result.fold(
      (failure) => emit(SessionError(failure: failure)),
      (r) => emit(SessionComplete(result: r)),
    );
  }

  void _onResult(_ResultReceived event, Emitter<SessionState> emit) {
    emit(SessionComplete(result: event.result));
  }

  void _onReset(SessionReset event, Emitter<SessionState> emit) {
    _flowSub?.cancel();
    emit(const SessionIdle());
  }

  /// Called externally by the presentation layer to push flow measurements.
  void pushFlowMeasurement(FlowMeasurement m) => add(_FlowReceived(m));

  /// Called when the firmware sends the FEV1/PEF result (GATT Indicate).
  void pushResult(SessionResultEntity r) => add(_ResultReceived(r));

  @override
  Future<void> close() {
    _flowSub?.cancel();
    return super.close();
  }
}
