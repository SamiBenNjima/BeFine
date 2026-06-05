import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../../../session/domain/entities/session_result_entity.dart';
import '../../data/repositories/history_repository_impl.dart';

final class GetSessionHistory {
  const GetSessionHistory(this._repository);
  final HistoryRepositoryImpl _repository;

  Future<Either<Failure, List<SessionResultEntity>>> call({
    DateTime? from,
    DateTime? to,
  }) =>
      _repository.getHistory(from: from, to: to);
}
