import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import '../../../../core/config/app_config.dart';
import '../../../../core/theme/app_theme.dart';
import '../bloc/session_bloc.dart';

/// Real-time flow chart — sliding window of AppConfig.flowChartWindowSec seconds.
class FlowChartWidget extends StatefulWidget {
  const FlowChartWidget({super.key});

  @override
  State<FlowChartWidget> createState() => _FlowChartWidgetState();
}

class _FlowChartWidgetState extends State<FlowChartWidget> {
  final _spots = <FlSpot>[];
  double _timeOffset = 0;

  @override
  Widget build(BuildContext context) {
    return BlocListener<SessionBloc, SessionState>(
      listener: (context, state) {
        if (state is SessionInhaling) {
          setState(() {
            _timeOffset += 0.02; // 50 Hz = 20 ms per sample
            _spots.add(FlSpot(_timeOffset, state.currentFlow.flowLpm));
            // Keep only the last N seconds
            final windowStart =
                _timeOffset - AppConfig.flowChartWindowSec;
            _spots.removeWhere((s) => s.x < windowStart);
          });
        }
      },
      child: LineChart(
        LineChartData(
          minY: 0,
          maxY: 600,
          minX: _timeOffset - AppConfig.flowChartWindowSec,
          maxX: _timeOffset,
          clipData: const FlClipData.all(),
          gridData: FlGridData(
            show: true,
            drawVerticalLine: false,
            getDrawingHorizontalLine: (_) =>
                FlLine(color: Colors.grey.shade200, strokeWidth: 1),
          ),
          titlesData: FlTitlesData(
            leftTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                reservedSize: 40,
                getTitlesWidget: (v, _) => Text(
                  v.toInt().toString(),
                  style: const TextStyle(fontSize: 10),
                ),
              ),
            ),
            bottomTitles: AxisTitles(
              sideTitles: SideTitles(
                showTitles: true,
                getTitlesWidget: (v, _) => Text(
                  '${v.toStringAsFixed(0)}s',
                  style: const TextStyle(fontSize: 10),
                ),
              ),
            ),
            rightTitles:
                const AxisTitles(sideTitles: SideTitles(showTitles: false)),
            topTitles:
                const AxisTitles(sideTitles: SideTitles(showTitles: false)),
          ),
          borderData: FlBorderData(show: false),
          lineBarsData: [
            LineChartBarData(
              spots: List<FlSpot>.from(_spots),
              isCurved: true,
              color: AppColors.primary,
              barWidth: 2.5,
              dotData: const FlDotData(show: false),
              belowBarData: BarAreaData(
                show: true,
                color: AppColors.primary.withOpacity(0.08),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
