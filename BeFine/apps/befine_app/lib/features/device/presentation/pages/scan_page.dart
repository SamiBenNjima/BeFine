import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:go_router/go_router.dart';
import '../../../../core/di/injection_container.dart';
import '../../../../core/theme/app_theme.dart';
import '../bloc/device_bloc.dart';
import '../widgets/device_tile.dart';

class ScanPage extends StatelessWidget {
  const ScanPage({super.key});

  @override
  Widget build(BuildContext context) {
    return BlocProvider(
      create: (_) => sl<DeviceBloc>()..add(const ScanStarted()),
      child: Scaffold(
        appBar: AppBar(title: const Text('Rechercher un appareil')),
        body: BlocConsumer<DeviceBloc, DeviceState>(
          listener: (context, state) {
            if (state is DeviceConnected) {
              context.go('/');
            }
          },
          builder: (context, state) {
            return switch (state) {
              DeviceScanning() => const Center(
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      CircularProgressIndicator(),
                      SizedBox(height: 16),
                      Text('Recherche en cours...'),
                    ],
                  ),
                ),
              DevicesFound(devices: final devices) => ListView.separated(
                  padding: const EdgeInsets.all(16),
                  itemCount: devices.length,
                  separatorBuilder: (_, __) => const SizedBox(height: 8),
                  itemBuilder: (ctx, i) => DeviceTile(
                    device: devices[i],
                    onTap: () => ctx
                        .read<DeviceBloc>()
                        .add(DeviceSelected(deviceId: devices[i].id)),
                  ),
                ),
              DeviceConnecting() => const Center(
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      CircularProgressIndicator(),
                      SizedBox(height: 16),
                      Text('Connexion...'),
                    ],
                  ),
                ),
              DeviceError(failure: final f) => Center(
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      const Icon(Icons.bluetooth_disabled,
                          size: 64, color: AppColors.error),
                      const SizedBox(height: 16),
                      Text(f.message,
                          textAlign: TextAlign.center),
                      const SizedBox(height: 16),
                      ElevatedButton(
                        onPressed: () => context
                            .read<DeviceBloc>()
                            .add(const ScanStarted()),
                        child: const Text('Réessayer'),
                      ),
                    ],
                  ),
                ),
              _ => const SizedBox.shrink(),
            };
          },
        ),
      ),
    );
  }
}
