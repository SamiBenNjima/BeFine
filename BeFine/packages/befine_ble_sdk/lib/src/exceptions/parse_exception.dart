class ParseException implements Exception {
  const ParseException({
    required this.characteristic,
    required this.message,
  });

  factory ParseException.malformedFrame({
    required String characteristic,
    required int expected,
    required int actual,
  }) =>
      ParseException(
        characteristic: characteristic,
        message: 'Malformed frame for $characteristic: '
            'expected $expected bytes, got $actual',
      );

  final String characteristic;
  final String message;

  @override
  String toString() => 'ParseException($characteristic): $message';
}
