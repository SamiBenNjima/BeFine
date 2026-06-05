import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../entities/session_result_entity.dart';

abstract interface class SessionRepository {
  Future<Either<Failure, void>> startSession();
  Future<Either<Failure, SessionResultEntity>> endSession();
  Stream<Either<Failure, FlowMeasurement>> get flowMeasurements;
  Stream<Either<Failure, EnvironmentData>> get environmentReadings;
}
