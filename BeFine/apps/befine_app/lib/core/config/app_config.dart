abstract final class AppConfig {
  static const Duration scanTimeout = Duration(seconds: 10);
  static const int reconnectMaxAttempts = 5;
  static const double flowChartWindowSec = 6.0;
  static const int holdDurationTargetSec = 10;
  static const double minPeakFlowLpm = 30.0;
  static const double flowOnsetLpm = 0.5;
  static const double flowEndLpm = 0.1;
}
