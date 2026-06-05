import 'package:equatable/equatable.dart';

sealed class Failure extends Equatable {
  const Failure({required this.message});
  final String message;
  @override
  List<Object> get props => [message];
}

final class BleFailure extends Failure {
  const BleFailure({required super.message});
}

final class StorageFailure extends Failure {
  const StorageFailure({required super.message});
}

final class ParseFailure extends Failure {
  const ParseFailure({required super.message});
}

final class PermissionFailure extends Failure {
  const PermissionFailure({required super.message});
}

final class UnknownFailure extends Failure {
  const UnknownFailure({required super.message});
}
