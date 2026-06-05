import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:go_router/go_router.dart';
import '../../../../core/di/injection_container.dart';
import '../../../../core/theme/app_theme.dart';
import '../bloc/session_bloc.dart';
import '../widgets/flow_chart_widget.dart';
import '../widgets/step_indicator.dart';
import '../widgets/technique_score_badge.dart';

class GuidedSessionPage extends StatelessWidget {
  const GuidedSessionPage({super.key});

  @override
  Widget build(BuildContext context) {
    return BlocProvider(
      create: (_) => sl<SessionBloc>()..add(const SessionStartRequested()),
      child: const _GuidedSessionView(),
    );
  }
}

class _GuidedSessionView extends StatelessWidget {
  const _GuidedSessionView();

  @override
  Widget build(BuildContext context) {
    return BlocConsumer<SessionBloc, SessionState>(
      listener: (context, state) {
        if (state is SessionComplete) {
          context.go('/session/summary', extra: state.result);
        }
      },
      builder: (context, state) {
        return Scaffold(
          appBar: AppBar(
            title: const Text('Session guidée'),
            leading: IconButton(
              icon: const Icon(Icons.close),
              onPressed: () {
                context.read<SessionBloc>().add(const SessionReset());
                context.pop();
              },
            ),
          ),
          body: SafeArea(
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: Column(
                children: [
                  StepIndicator(currentState: state),
                  const SizedBox(height: 24),
                  Expanded(child: _buildBody(context, state)),
                ],
              ),
            ),
          ),
        );
      },
    );
  }

  Widget _buildBody(BuildContext context, SessionState state) {
    return switch (state) {
      SessionIdle() => const Center(child: CircularProgressIndicator()),
      SessionShaking() => _StepCard(
          step: 1,
          icon: Icons.vibration,
          title: 'Agitez le dispositif',
          subtitle: 'Secouez pendant 5 secondes',
          color: AppColors.primary,
        ),
      SessionInserting() => _StepCard(
          step: 2,
          icon: Icons.medication_outlined,
          title: 'Insérez la cartouche',
          subtitle: 'Insérez fermement le spray',
          color: AppColors.secondary,
        ),
      SessionInhaling(currentFlow: final m) => Column(
          children: [
            _StepCard(
              step: 3,
              icon: Icons.air,
              title: 'Inhaler profondément',
              subtitle: '${m.flowLpm.toStringAsFixed(1)} L/min',
              color: AppColors.scoreA,
            ),
            const SizedBox(height: 16),
            const Expanded(child: FlowChartWidget()),
          ],
        ),
      SessionHolding(remainingSec: final s) => _StepCard(
          step: 4,
          icon: Icons.timer_outlined,
          title: 'Retenez la respiration',
          subtitle: '$s secondes',
          color: AppColors.scoreC,
        ),
      SessionError(failure: final f) => Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              const Icon(Icons.error_outline, size: 64, color: AppColors.error),
              const SizedBox(height: 16),
              Text(f.message, textAlign: TextAlign.center),
              const SizedBox(height: 16),
              ElevatedButton(
                onPressed: () => context
                    .read<SessionBloc>()
                    .add(const SessionStartRequested()),
                child: const Text('Réessayer'),
              ),
            ],
          ),
        ),
      _ => const SizedBox.shrink(),
    };
  }
}

class _StepCard extends StatelessWidget {
  const _StepCard({
    required this.step,
    required this.icon,
    required this.title,
    required this.subtitle,
    required this.color,
  });

  final int step;
  final IconData icon;
  final String title;
  final String subtitle;
  final Color color;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            CircleAvatar(
              radius: 40,
              backgroundColor: color.withOpacity(0.15),
              child: Icon(icon, size: 40, color: color),
            ),
            const SizedBox(height: 16),
            Text(title,
                style: Theme.of(context).textTheme.headlineMedium,
                textAlign: TextAlign.center),
            const SizedBox(height: 8),
            Text(subtitle,
                style: Theme.of(context)
                    .textTheme
                    .bodyLarge
                    ?.copyWith(color: color),
                textAlign: TextAlign.center),
          ],
        ),
      ),
    );
  }
}
