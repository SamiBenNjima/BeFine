sealed class BefineConnectionState {
  const BefineConnectionState();
}

final class Disconnected extends BefineConnectionState {
  const Disconnected();
}

final class Connecting extends BefineConnectionState {
  const Connecting();
}

final class Connected extends BefineConnectionState {
  const Connected();
}

final class Reconnecting extends BefineConnectionState {
  const Reconnecting({required this.attempt});
  final int attempt;
}

final class ConnectionFailed extends BefineConnectionState {
  const ConnectionFailed({required this.reason});
  final String reason;
}
