import 'package:flutter/foundation.dart';

@immutable
class EnvironmentData {
  const EnvironmentData({
    required this.temperatureC,
    required this.humidityPct,
    required this.timestamp,
  });

  final double temperatureC;
  final double humidityPct;
  final DateTime timestamp;

  EnvironmentData copyWith({
    double? temperatureC,
    double? humidityPct,
    DateTime? timestamp,
  }) =>
      EnvironmentData(
        temperatureC: temperatureC ?? this.temperatureC,
        humidityPct: humidityPct ?? this.humidityPct,
        timestamp: timestamp ?? this.timestamp,
      );

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is EnvironmentData &&
          temperatureC == other.temperatureC &&
          humidityPct == other.humidityPct &&
          timestamp == other.timestamp;

  @override
  int get hashCode => Object.hash(temperatureC, humidityPct, timestamp);
}
