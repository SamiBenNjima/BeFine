import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';

class SessionDetailPage extends StatelessWidget {
  const SessionDetailPage({super.key, required this.sessionId});
  final String sessionId;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Session #$sessionId')),
      body: Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Détails de la session',
                style: Theme.of(context).textTheme.headlineMedium),
            const SizedBox(height: 24),
            OutlinedButton.icon(
              onPressed: () => context.go('/export/$sessionId'),
              icon: const Icon(Icons.share),
              label: const Text('Exporter'),
            ),
          ],
        ),
      ),
    );
  }
}
