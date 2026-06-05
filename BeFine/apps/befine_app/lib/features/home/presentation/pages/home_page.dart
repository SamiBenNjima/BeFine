import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import '../../../../core/theme/app_theme.dart';

class HomePage extends StatelessWidget {
  const HomePage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('BeFine'),
        actions: [
          IconButton(
            icon: const Icon(Icons.bluetooth_searching),
            tooltip: 'Connecter un appareil',
            onPressed: () => context.push('/scan'),
          ),
        ],
      ),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(24),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // ── Connection status card ─────────────────────────────────
              Card(
                color: AppColors.primaryLight.withOpacity(0.15),
                child: ListTile(
                  leading: const Icon(Icons.bluetooth_connected,
                      color: AppColors.primary),
                  title: const Text('Aucun appareil connecté'),
                  subtitle: const Text('Appuyez pour scanner'),
                  trailing:
                      const Icon(Icons.chevron_right, color: AppColors.primary),
                  onTap: () => context.push('/scan'),
                ),
              ),
              const SizedBox(height: 32),
              // ── Quick start button ─────────────────────────────────────
              ElevatedButton.icon(
                onPressed: () => context.push('/session'),
                style: ElevatedButton.styleFrom(
                  padding: const EdgeInsets.symmetric(vertical: 20),
                  textStyle: const TextStyle(fontSize: 18),
                ),
                icon: const Icon(Icons.play_arrow, size: 28),
                label: const Text('Démarrer une session'),
              ),
              const SizedBox(height: 16),
              OutlinedButton.icon(
                onPressed: () => context.go('/history'),
                icon: const Icon(Icons.history),
                label: const Text('Voir l\'historique'),
              ),
              const Spacer(),
              // ── Last session summary ───────────────────────────────────
              Text('Dernière session',
                  style: Theme.of(context).textTheme.titleLarge),
              const SizedBox(height: 8),
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(16),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceAround,
                    children: [
                      _StatChip(
                          label: 'FEV1', value: '--', unit: 'L'),
                      _StatChip(
                          label: 'PEF', value: '--', unit: 'L/min'),
                      _StatChip(
                          label: 'Score', value: '--', unit: ''),
                    ],
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _StatChip extends StatelessWidget {
  const _StatChip({
    required this.label,
    required this.value,
    required this.unit,
  });

  final String label;
  final String value;
  final String unit;

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Text(label,
            style: Theme.of(context)
                .textTheme
                .bodyMedium
                ?.copyWith(color: Colors.grey)),
        const SizedBox(height: 4),
        Text(value,
            style: Theme.of(context)
                .textTheme
                .headlineMedium
                ?.copyWith(color: AppColors.primary)),
        if (unit.isNotEmpty)
          Text(unit,
              style: Theme.of(context)
                  .textTheme
                  .bodyMedium
                  ?.copyWith(color: Colors.grey, fontSize: 11)),
      ],
    );
  }
}
