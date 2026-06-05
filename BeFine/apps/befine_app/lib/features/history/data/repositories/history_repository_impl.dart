import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../../../session/domain/entities/session_result_entity.dart';
import 'package:befine_ble_sdk/befine_ble_sdk.dart';

// Stub in-memory repository — replace with Drift database in production
class HistoryRepositoryImpl {
  final _sessions = <SessionResultEntity>[];

  void addSession(SessionResultEntity r) => _sessions.add(r);

  Future<Either<Failure, List<SessionResultEntity>>> getHistory({
    DateTime? from,
    DateTime? to,
  }) async {
    try {
      var filtered = List<SessionResultEntity>.from(_sessions);
      if (from != null) {
        filtered =
            filtered.where((s) => s.recordedAt.isAfter(from)).toList();
      }
      if (to != null) {
        filtered =
            filtered.where((s) => s.recordedAt.isBefore(to)).toList();
      }
      filtered.sort((a, b) => b.recordedAt.compareTo(a.recordedAt));
      return Right(filtered);
    } on Exception catch (e) {
      return Left(StorageFailure(message: e.toString()));
    }
  }
}
