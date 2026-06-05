import 'package:flutter/foundation.dart';

@immutable
class FlowMeasurement {
  const FlowMeasurement({
    required this.flowLpm,
    required this.isInhaling,
    required this.isExhaling,
    required this.timestamp,
  });

  final double flowLpm;
  final bool isInhaling;
  final bool isExhaling;
  final DateTime timestamp;

  FlowMeasurement copyWith({
    double? flowLpm,
    bool? isInhaling,
    bool? isExhaling,
    DateTime? timestamp,
  }) =>
      FlowMeasurement(
        flowLpm: flowLpm ?? this.flowLpm,
        isInhaling: isInhaling ?? this.isInhaling,
        isExhaling: isExhaling ?? this.isExhaling,
        timestamp: timestamp ?? this.timestamp,
      );

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is FlowMeasurement &&
          flowLpm == other.flowLpm &&
          isInhaling == other.isInhaling &&
          isExhaling == other.isExhaling &&
          timestamp == other.timestamp;

  @override
  int get hashCode =>
      Object.hash(flowLpm, isInhaling, isExhaling, timestamp);

  @override
  String toString() =>
      'FlowMeasurement(flowLpm: $flowLpm, isInhaling: $isInhaling, '
      'isExhaling: $isExhaling, ts: $timestamp)';
}
