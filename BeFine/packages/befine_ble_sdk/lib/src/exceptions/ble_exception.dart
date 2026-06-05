enum BleErrorCode { connectionFailed, timeout, notSupported, permissionDenied }

class BleException implements Exception {
  const BleException({required this.message, required this.code});

  factory BleException.connectionFailed([String? details]) => BleException(
        message: 'BLE connection failed${details != null ? ': $details' : ''}',
        code: BleErrorCode.connectionFailed,
      );

  factory BleException.timeout() => const BleException(
        message: 'BLE connection timed out',
        code: BleErrorCode.timeout,
      );

  factory BleException.notSupported() => const BleException(
        message: 'BLE not supported on this device',
        code: BleErrorCode.notSupported,
      );

  final String message;
  final BleErrorCode code;

  @override
  String toString() => 'BleException(${code.name}): $message';
}
