import 'package:equatable/equatable.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import '../../domain/entities/device.dart';
import '../../domain/usecases/scan_devices.dart';
import '../../domain/usecases/connect_device.dart';
import '../../../../core/errors/failures.dart';

// ── Events ────────────────────────────────────────────────────────────────
sealed class DeviceEvent extends Equatable {
  const DeviceEvent();
  @override
  List<Object?> get props => [];
}

final class ScanStarted extends DeviceEvent {
  const ScanStarted();
}

final class DeviceSelected extends DeviceEvent {
  const DeviceSelected({required this.deviceId});
  final String deviceId;
  @override
  List<Object?> get props => [deviceId];
}

// ── States ────────────────────────────────────────────────────────────────
sealed class DeviceState extends Equatable {
  const DeviceState();
  @override
  List<Object?> get props => [];
}

final class DeviceIdle extends DeviceState {
  const DeviceIdle();
}

final class DeviceScanning extends DeviceState {
  const DeviceScanning();
}

final class DevicesFound extends DeviceState {
  const DevicesFound({required this.devices});
  final List<Device> devices;
  @override
  List<Object?> get props => [devices];
}

final class DeviceConnecting extends DeviceState {
  const DeviceConnecting({required this.deviceId});
  final String deviceId;
  @override
  List<Object?> get props => [deviceId];
}

final class DeviceConnected extends DeviceState {
  const DeviceConnected({required this.device});
  final Device device;
  @override
  List<Object?> get props => [device];
}

final class DeviceError extends DeviceState {
  const DeviceError({required this.failure});
  final Failure failure;
  @override
  List<Object?> get props => [failure];
}

// ── BLoC ──────────────────────────────────────────────────────────────────
class DeviceBloc extends Bloc<DeviceEvent, DeviceState> {
  DeviceBloc({
    required ScanDevices scanDevices,
    required ConnectDevice connectDevice,
  })  : _scan = scanDevices,
        _connect = connectDevice,
        super(const DeviceIdle()) {
    on<ScanStarted>(_onScanStarted);
    on<DeviceSelected>(_onDeviceSelected);
  }

  final ScanDevices _scan;
  final ConnectDevice _connect;

  Future<void> _onScanStarted(
    ScanStarted event,
    Emitter<DeviceState> emit,
  ) async {
    emit(const DeviceScanning());
    final result = await _scan();
    result.fold(
      (failure) => emit(DeviceError(failure: failure)),
      (devices) => devices.isEmpty
          ? emit(DeviceError(
              failure: const BleFailure(message: 'No BeFine device found')))
          : emit(DevicesFound(devices: devices)),
    );
  }

  Future<void> _onDeviceSelected(
    DeviceSelected event,
    Emitter<DeviceState> emit,
  ) async {
    emit(DeviceConnecting(deviceId: event.deviceId));
    final result = await _connect(event.deviceId);
    result.fold(
      (failure) => emit(DeviceError(failure: failure)),
      (device) => emit(DeviceConnected(device: device)),
    );
  }
}
