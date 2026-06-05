import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../../domain/entities/device.dart';
import '../../domain/repositories/device_repository.dart';
import '../datasources/ble_device_datasource.dart';

final class DeviceRepositoryImpl implements DeviceRepository {
  const DeviceRepositoryImpl({required BleDeviceDataSource bleDataSource})
      : _ble = bleDataSource;

  final BleDeviceDataSource _ble;

  @override
  Future<Either<Failure, List<Device>>> scan() async {
    try {
      return Right(await _ble.scan());
    } on BleException catch (e) {
      return Left(BleFailure(message: e.message));
    } on Exception catch (e) {
      return Left(UnknownFailure(message: e.toString()));
    }
  }

  @override
  Future<Either<Failure, Device>> connect(String deviceId) async {
    try {
      return Right(await _ble.connect(deviceId));
    } on BleException catch (e) {
      return Left(BleFailure(message: e.message));
    }
  }

  @override
  Future<Either<Failure, void>> disconnect(String deviceId) async {
    try {
      await _ble.disconnect(deviceId);
      return const Right(null);
    } on BleException catch (e) {
      return Left(BleFailure(message: e.message));
    }
  }
}
