import 'package:bloc_test/bloc_test.dart';
import 'package:dartz/dartz.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:mocktail/mocktail.dart';

import 'package:befine_app/core/errors/failures.dart';
import 'package:befine_app/features/device/domain/entities/device.dart';
import 'package:befine_app/features/device/domain/usecases/scan_devices.dart';
import 'package:befine_app/features/device/domain/usecases/connect_device.dart';
import 'package:befine_app/features/device/presentation/bloc/device_bloc.dart';

class MockScanDevices extends Mock implements ScanDevices {}
class MockConnectDevice extends Mock implements ConnectDevice {}

const _fakeDevice = Device(id: 'AA:BB:CC', name: 'BeFine-001', rssi: -65);

void main() {
  late MockScanDevices mockScan;
  late MockConnectDevice mockConnect;

  setUp(() {
    mockScan = MockScanDevices();
    mockConnect = MockConnectDevice();
  });

  group('DeviceBloc', () {
    blocTest<DeviceBloc, DeviceState>(
      'emits [Scanning, DevicesFound] when scan returns devices',
      build: () {
        when(() => mockScan())
            .thenAnswer((_) async => const Right([_fakeDevice]));
        return DeviceBloc(scanDevices: mockScan, connectDevice: mockConnect);
      },
      act: (b) => b.add(const ScanStarted()),
      expect: () => [
        isA<DeviceScanning>(),
        isA<DevicesFound>(),
      ],
    );

    blocTest<DeviceBloc, DeviceState>(
      'emits [Scanning, DeviceError] when scan returns empty',
      build: () {
        when(() => mockScan())
            .thenAnswer((_) async => const Right([]));
        return DeviceBloc(scanDevices: mockScan, connectDevice: mockConnect);
      },
      act: (b) => b.add(const ScanStarted()),
      expect: () => [
        isA<DeviceScanning>(),
        isA<DeviceError>(),
      ],
    );

    blocTest<DeviceBloc, DeviceState>(
      'emits [Connecting, Connected] when connect succeeds',
      build: () {
        when(() => mockConnect('AA:BB:CC'))
            .thenAnswer((_) async => const Right(_fakeDevice));
        return DeviceBloc(scanDevices: mockScan, connectDevice: mockConnect);
      },
      act: (b) =>
          b.add(const DeviceSelected(deviceId: 'AA:BB:CC')),
      expect: () => [
        isA<DeviceConnecting>(),
        isA<DeviceConnected>(),
      ],
    );

    blocTest<DeviceBloc, DeviceState>(
      'emits [Connecting, DeviceError] when connect fails',
      build: () {
        when(() => mockConnect('AA:BB:CC')).thenAnswer((_) async =>
            const Left(BleFailure(message: 'Connection refused')));
        return DeviceBloc(scanDevices: mockScan, connectDevice: mockConnect);
      },
      act: (b) =>
          b.add(const DeviceSelected(deviceId: 'AA:BB:CC')),
      expect: () => [
        isA<DeviceConnecting>(),
        isA<DeviceError>(),
      ],
    );
  });
}
