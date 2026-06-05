import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../entities/device.dart';
import '../repositories/device_repository.dart';

final class ScanDevices {
  const ScanDevices(this._repository);
  final DeviceRepository _repository;

  Future<Either<Failure, List<Device>>> call() => _repository.scan();
}
