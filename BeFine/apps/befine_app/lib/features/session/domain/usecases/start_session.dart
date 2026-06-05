import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../repositories/session_repository.dart';

final class StartSession {
  const StartSession(this._repository);
  final SessionRepository _repository;

  Future<Either<Failure, void>> call() => _repository.startSession();
}
