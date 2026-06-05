import 'dart:math';

class ReconnectPolicy {
  const ReconnectPolicy({
    this.maxAttempts = 5,
    this.initialDelay = const Duration(milliseconds: 500),
    this.maxDelay = const Duration(seconds: 30),
    this.backoffFactor = 2.0,
    this.jitterFraction = 0.1,
  });

  final int maxAttempts;
  final Duration initialDelay;
  final Duration maxDelay;
  final double backoffFactor;
  final double jitterFraction;

  Duration delayFor(int attempt) {
    final base = initialDelay * pow(backoffFactor, attempt);
    final capped = base > maxDelay ? maxDelay : base;
    final jitter = capped * jitterFraction * (Random().nextDouble() - 0.5);
    return capped + jitter;
  }
}
