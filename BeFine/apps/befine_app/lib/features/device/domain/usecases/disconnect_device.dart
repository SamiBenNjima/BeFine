import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../repositories/device_repository.dart';

final class DisconnectDevice {
  const DisconnectDevice(this._repository);
  final DeviceRepository _repository;

  Future<Either<Failure, void>> call(String deviceId) =>
      _repository.disconnect(deviceId);
}
