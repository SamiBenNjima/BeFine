import 'package:dartz/dartz.dart';
import '../../../../core/errors/failures.dart';
import '../entities/device.dart';

abstract interface class DeviceRepository {
  Future<Either<Failure, List<Device>>> scan();
  Future<Either<Failure, Device>> connect(String deviceId);
  Future<Either<Failure, void>> disconnect(String deviceId);
}
