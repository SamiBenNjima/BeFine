import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../entities/session_result_entity.dart';
import '../repositories/session_repository.dart';

final class EndSession {
  const EndSession(this._repository);
  final SessionRepository _repository;

  Future<Either<Failure, SessionResultEntity>> call() =>
      _repository.endSession();
}
