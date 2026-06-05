import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../entities/device.dart';
import '../repositories/device_repository.dart';

final class ConnectDevice {
  const ConnectDevice(this._repository);
  final DeviceRepository _repository;

  Future<Either<Failure, Device>> call(String deviceId) =>
      _repository.connect(deviceId);
}
