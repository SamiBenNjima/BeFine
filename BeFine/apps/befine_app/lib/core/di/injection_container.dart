import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:get_it/get_it.dart';
import '../config/app_config.dart';
import '../../features/device/data/datasources/ble_device_datasource.dart';
import '../../features/device/data/repositories/device_repository_impl.dart';
import '../../features/device/domain/repositories/device_repository.dart';
import '../../features/device/domain/usecases/scan_devices.dart';
import '../../features/device/domain/usecases/connect_device.dart';
import '../../features/device/domain/usecases/disconnect_device.dart';
import '../../features/device/presentation/bloc/device_bloc.dart';
import '../../features/session/data/datasources/ble_session_datasource.dart';
import '../../features/session/data/repositories/session_repository_impl.dart';
import '../../features/session/domain/repositories/session_repository.dart';
import '../../features/session/domain/usecases/start_session.dart';
import '../../features/session/domain/usecases/end_session.dart';
import '../../features/session/presentation/bloc/session_bloc.dart';
import '../../features/history/data/repositories/history_repository_impl.dart';
import '../../features/history/domain/usecases/get_session_history.dart';
import '../../features/history/presentation/bloc/history_bloc.dart';

final sl = GetIt.instance;

Future<void> configureDependencies() async {
  // ── BLE SDK ──────────────────────────────────────────────────────────────
  sl.registerLazySingleton<BleManager>(
    () => BleManager(
      policy: ReconnectPolicy(
        maxAttempts: AppConfig.reconnectMaxAttempts,
      ),
    ),
  );
  sl.registerLazySingleton<BleScanner>(() => BleScanner());

  // ── Data sources ─────────────────────────────────────────────────────────
  sl.registerLazySingleton<BleDeviceDataSource>(
    () => BleDeviceDataSourceImpl(scanner: sl(), manager: sl()),
  );
  sl.registerLazySingleton<BleSessionDataSource>(
    () => BleSessionDataSourceImpl(manager: sl()),
  );

  // ── Repositories ─────────────────────────────────────────────────────────
  sl.registerLazySingleton<DeviceRepository>(
    () => DeviceRepositoryImpl(bleDataSource: sl()),
  );
  sl.registerLazySingleton<SessionRepository>(
    () => SessionRepositoryImpl(bleDataSource: sl()),
  );
  sl.registerLazySingleton<HistoryRepositoryImpl>(
    () => HistoryRepositoryImpl(),
  );

  // ── Use cases ─────────────────────────────────────────────────────────────
  sl.registerFactory(() => ScanDevices(sl()));
  sl.registerFactory(() => ConnectDevice(sl()));
  sl.registerFactory(() => DisconnectDevice(sl()));
  sl.registerFactory(() => StartSession(sl()));
  sl.registerFactory(() => EndSession(sl()));
  sl.registerFactory(() => GetSessionHistory(sl()));

  // ── BLoCs ─────────────────────────────────────────────────────────────────
  sl.registerFactory(
    () => DeviceBloc(scanDevices: sl(), connectDevice: sl()),
  );
  sl.registerFactory(
    () => SessionBloc(startSession: sl(), endSession: sl()),
  );
  sl.registerFactory(
    () => HistoryBloc(getSessionHistory: sl()),
  );
}
