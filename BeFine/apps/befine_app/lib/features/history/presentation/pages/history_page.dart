import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:go_router/go_router.dart';
import '../../../../core/di/injection_container.dart';
import '../../../../core/theme/app_theme.dart';
import '../../../session/domain/entities/session_result_entity.dart';
import '../../../session/presentation/widgets/technique_score_badge.dart';
import '../bloc/history_bloc.dart';

class HistoryPage extends StatelessWidget {
  const HistoryPage({super.key});

  @override
  Widget build(BuildContext context) {
    return BlocProvider(
      create: (_) =>
          sl<HistoryBloc>()..add(const HistoryLoadRequested()),
      child: Scaffold(
        appBar: AppBar(title: const Text('Historique')),
        body: BlocBuilder<HistoryBloc, HistoryState>(
          builder: (context, state) => switch (state) {
            HistoryLoading() =>
              const Center(child: CircularProgressIndicator()),
            HistoryLoaded(sessions: final s) when s.isEmpty => const Center(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Icon(Icons.history_toggle_off,
                        size: 64, color: Colors.grey),
                    SizedBox(height: 16),
                    Text('Aucune session enregistrée'),
                  ],
                ),
              ),
            HistoryLoaded(sessions: final sessions) => ListView.separated(
                padding: const EdgeInsets.all(16),
                itemCount: sessions.length,
                separatorBuilder: (_, __) => const SizedBox(height: 8),
                itemBuilder: (ctx, i) =>
                    _SessionCard(session: sessions[i]),
              ),
            HistoryError(failure: final f) => Center(
                child: Text(f.message)),
            _ => const SizedBox.shrink(),
          },
        ),
      ),
    );
  }
}

class _SessionCard extends StatelessWidget {
  const _SessionCard({required this.session});
  final SessionResultEntity session;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        leading: TechniqueScoreBadge(score: session.score, size: 40),
        title: Text(
          '${session.recordedAt.day}/${session.recordedAt.month}/${session.recordedAt.year}',
          style: Theme.of(context).textTheme.titleLarge,
        ),
        subtitle: Text(
          'FEV1: ${session.fev1L.toStringAsFixed(2)} L  '
          'PEF: ${session.pefLpm.toStringAsFixed(0)} L/min',
        ),
        trailing: const Icon(Icons.chevron_right),
        onTap: () => context.go('/history/${session.sessionId}'),
      ),
    );
  }
}
