import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import '../../../../core/theme/app_theme.dart';
import '../../domain/entities/session_result_entity.dart';
import '../widgets/technique_score_badge.dart';

class SessionSummaryPage extends StatelessWidget {
  const SessionSummaryPage({super.key, required this.result});
  final SessionResultEntity result;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Résultats')),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(24),
          child: Column(
            children: [
              TechniqueScoreBadge(score: result.score, size: 80),
              const SizedBox(height: 32),
              _MetricRow(
                label: 'FEV1',
                value: '${result.fev1L.toStringAsFixed(2)} L',
                color: AppColors.primary,
              ),
              const Divider(),
              _MetricRow(
                label: 'PEF',
                value: '${result.pefLpm.toStringAsFixed(0)} L/min',
                color: AppColors.secondary,
              ),
              const Divider(),
              _MetricRow(
                label: 'Durée',
                value: '${(result.durationMs / 1000).toStringAsFixed(1)} s',
                color: AppColors.onBackground,
              ),
              const Spacer(),
              Row(
                children: [
                  Expanded(
                    child: OutlinedButton.icon(
                      onPressed: () =>
                          context.go('/export/${result.sessionId}'),
                      icon: const Icon(Icons.share),
                      label: const Text('Exporter'),
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: ElevatedButton(
                      onPressed: () => context.go('/'),
                      child: const Text('Terminer'),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _MetricRow extends StatelessWidget {
  const _MetricRow({
    required this.label,
    required this.value,
    required this.color,
  });

  final String label;
  final String value;
  final Color color;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 12),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: Theme.of(context).textTheme.titleLarge),
          Text(
            value,
            style: Theme.of(context)
                .textTheme
                .titleLarge
                ?.copyWith(color: color, fontWeight: FontWeight.bold),
          ),
        ],
      ),
    );
  }
}
