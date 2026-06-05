import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import '../../features/home/presentation/pages/home_page.dart';
import '../../features/device/presentation/pages/scan_page.dart';
import '../../features/device/presentation/pages/device_detail_page.dart';
import '../../features/session/presentation/pages/guided_session_page.dart';
import '../../features/session/presentation/pages/session_summary_page.dart';
import '../../features/history/presentation/pages/history_page.dart';
import '../../features/history/presentation/pages/session_detail_page.dart';
import '../../features/export/presentation/pages/export_page.dart';
import '../../features/settings/presentation/pages/settings_page.dart';
import '../../../features/session/domain/entities/session_result_entity.dart';

final appRouter = GoRouter(
  initialLocation: '/',
  debugLogDiagnostics: false,
  routes: [
    // ── Bottom nav shell ──────────────────────────────────────────────────
    ShellRoute(
      builder: (context, state, child) =>
          _ScaffoldWithNavBar(child: child),
      routes: [
        GoRoute(
          path: '/',
          builder: (_, __) => const HomePage(),
        ),
        GoRoute(
          path: '/history',
          builder: (_, __) => const HistoryPage(),
          routes: [
            GoRoute(
              path: ':id',
              builder: (_, state) =>
                  SessionDetailPage(sessionId: state.pathParameters['id']!),
            ),
          ],
        ),
        GoRoute(
          path: '/settings',
          builder: (_, __) => const SettingsPage(),
        ),
      ],
    ),

    // ── Modal routes (no bottom nav) ──────────────────────────────────────
    GoRoute(
      path: '/scan',
      builder: (_, __) => const ScanPage(),
    ),
    GoRoute(
      path: '/device/:id',
      builder: (_, state) =>
          DeviceDetailPage(deviceId: state.pathParameters['id']!),
    ),
    GoRoute(
      path: '/session',
      builder: (_, __) => const GuidedSessionPage(),
    ),
    GoRoute(
      path: '/session/summary',
      builder: (_, state) => SessionSummaryPage(
        result: state.extra! as SessionResultEntity,
      ),
    ),
    GoRoute(
      path: '/export/:id',
      builder: (_, state) =>
          ExportPage(sessionId: state.pathParameters['id']!),
    ),
  ],
);

// ── Bottom navigation scaffold ─────────────────────────────────────────────
class _ScaffoldWithNavBar extends StatelessWidget {
  const _ScaffoldWithNavBar({required this.child});
  final Widget child;

  static const _tabs = [
    (path: '/', icon: Icons.home_outlined, label: 'Accueil'),
    (path: '/history', icon: Icons.history_outlined, label: 'Historique'),
    (path: '/settings', icon: Icons.settings_outlined, label: 'Réglages'),
  ];

  @override
  Widget build(BuildContext context) {
    final location = GoRouterState.of(context).uri.path;
    final idx = _tabs.indexWhere((t) => location.startsWith(t.path) && t.path != '/'
        ? true
        : location == t.path);
    final currentIdx = idx < 0 ? 0 : idx;

    return Scaffold(
      body: child,
      bottomNavigationBar: NavigationBar(
        selectedIndex: currentIdx,
        onDestinationSelected: (i) => context.go(_tabs[i].path),
        destinations: _tabs
            .map((t) => NavigationDestination(
                  icon: Icon(t.icon),
                  label: t.label,
                ))
            .toList(),
      ),
    );
  }
}
