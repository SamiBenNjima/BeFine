import 'dart:io';
import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';
import 'package:share_plus/share_plus.dart';
import 'package:csv/csv.dart';

class ExportPage extends StatelessWidget {
  const ExportPage({super.key, required this.sessionId});
  final String sessionId;

  Future<void> _exportCsv(BuildContext context) async {
    // Stub — replace with actual session data from repository
    final rows = [
      ['timestamp_ms', 'flow_lpm'],
      ['1000', '0.0'],
      ['1020', '12.5'],
      ['1040', '45.2'],
    ];
    final csv = const ListToCsvConverter().convert(rows);
    final dir = await getTemporaryDirectory();
    final file = File('${dir.path}/session_$sessionId.csv');
    await file.writeAsString(csv);
    await Share.shareXFiles(
      [XFile(file.path)],
      subject: 'BeFine Session $sessionId',
    );
  }

  Future<void> _exportJson(BuildContext context) async {
    final json = '{"sessionId":$sessionId,'
        '"fev1":1.23,"pef":300.0,"score":"A"}';
    final dir = await getTemporaryDirectory();
    final file = File('${dir.path}/session_$sessionId.json');
    await file.writeAsString(json);
    await Share.shareXFiles(
      [XFile(file.path)],
      subject: 'BeFine Session $sessionId',
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Exporter')),
      body: Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text('Session #$sessionId',
                style: Theme.of(context).textTheme.headlineMedium),
            const SizedBox(height: 32),
            _ExportButton(
              icon: Icons.table_chart_outlined,
              label: 'Exporter en CSV',
              subtitle: 'Données brutes de flux',
              onTap: () => _exportCsv(context),
            ),
            const SizedBox(height: 16),
            _ExportButton(
              icon: Icons.code,
              label: 'Exporter en JSON',
              subtitle: 'Compatible EHR / intégration',
              onTap: () => _exportJson(context),
            ),
            const SizedBox(height: 16),
            _ExportButton(
              icon: Icons.picture_as_pdf_outlined,
              label: 'Exporter en PDF',
              subtitle: 'Rapport formaté pour le médecin',
              onTap: () => ScaffoldMessenger.of(context).showSnackBar(
                const SnackBar(content: Text('PDF export — disponible en v1.1')),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class _ExportButton extends StatelessWidget {
  const _ExportButton({
    required this.icon,
    required this.label,
    required this.subtitle,
    required this.onTap,
  });

  final IconData icon;
  final String label;
  final String subtitle;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        leading: Icon(icon, size: 32),
        title: Text(label,
            style: Theme.of(context).textTheme.titleLarge),
        subtitle: Text(subtitle),
        trailing: const Icon(Icons.chevron_right),
        onTap: onTap,
      ),
    );
  }
}
