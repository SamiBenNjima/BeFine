import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../../domain/entities/session_result_entity.dart';
import '../../domain/repositories/session_repository.dart';
import '../datasources/ble_session_datasource.dart';

final class SessionRepositoryImpl implements SessionRepository {
  const SessionRepositoryImpl({required BleSessionDataSource bleDataSource})
      : _ble = bleDataSource;

  final BleSessionDataSource _ble;

  @override
  Future<Either<Failure, void>> startSession() async {
    try {
      await _ble.sendStart();
      return const Right(null);
    } on BleException catch (e) {
      return Left(BleFailure(message: e.message));
    }
  }

  @override
  Future<Either<Failure, SessionResultEntity>> endSession() async {
    try {
      await _ble.sendStop();
      final result = await _ble.resultStream.first;
      return Right(SessionResultEntity.fromSdkResult(result));
    } on BleException catch (e) {
      return Left(BleFailure(message: e.message));
    }
  }

  @override
  Stream<Either<Failure, FlowMeasurement>> get flowMeasurements =>
      _ble.flowStream.map<Either<Failure, FlowMeasurement>>(Right.new);

  @override
  Stream<Either<Failure, EnvironmentData>> get environmentReadings =>
      _ble.environmentStream.map<Either<Failure, EnvironmentData>>(Right.new);
}
