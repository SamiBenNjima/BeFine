import 'package:equatable/equatable.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import '../../../../core/errors/failures.dart';
import '../../../session/domain/entities/session_result_entity.dart';
import '../../domain/usecases/get_session_history.dart';

// ── Events ─────────────────────────────────────────────────────────────────
sealed class HistoryEvent extends Equatable {
  const HistoryEvent();
  @override
  List<Object?> get props => [];
}

final class HistoryLoadRequested extends HistoryEvent {
  const HistoryLoadRequested({this.from, this.to});
  final DateTime? from;
  final DateTime? to;
  @override
  List<Object?> get props => [from, to];
}

// ── States ──────────────────────────────────────────────────────────────────
sealed class HistoryState extends Equatable {
  const HistoryState();
  @override
  List<Object?> get props => [];
}

final class HistoryInitial extends HistoryState {
  const HistoryInitial();
}

final class HistoryLoading extends HistoryState {
  const HistoryLoading();
}

final class HistoryLoaded extends HistoryState {
  const HistoryLoaded({required this.sessions});
  final List<SessionResultEntity> sessions;
  @override
  List<Object?> get props => [sessions];
}

final class HistoryError extends HistoryState {
  const HistoryError({required this.failure});
  final Failure failure;
  @override
  List<Object?> get props => [failure];
}

// ── BLoC ────────────────────────────────────────────────────────────────────
class HistoryBloc extends Bloc<HistoryEvent, HistoryState> {
  HistoryBloc({required GetSessionHistory getSessionHistory})
      : _getHistory = getSessionHistory,
        super(const HistoryInitial()) {
    on<HistoryLoadRequested>(_onLoad);
  }

  final GetSessionHistory _getHistory;

  Future<void> _onLoad(
    HistoryLoadRequested event,
    Emitter<HistoryState> emit,
  ) async {
    emit(const HistoryLoading());
    final result =
        await _getHistory(from: event.from, to: event.to);
    result.fold(
      (f) => emit(HistoryError(failure: f)),
      (sessions) => emit(HistoryLoaded(sessions: sessions)),
    );
  }
}
