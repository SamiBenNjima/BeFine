import 'package:flutter/material.dart';
import '../../../../core/theme/app_theme.dart';
import '../bloc/session_bloc.dart';

class StepIndicator extends StatelessWidget {
  const StepIndicator({super.key, required this.currentState});
  final SessionState currentState;

  int get _currentStep => switch (currentState) {
        SessionShaking() => 0,
        SessionInserting() => 1,
        SessionInhaling() => 2,
        SessionHolding() => 3,
        SessionComplete() => 4,
        _ => -1,
      };

  static const _labels = ['Agiter', 'Insérer', 'Inhaler', 'Retenir'];

  @override
  Widget build(BuildContext context) {
    return Row(
      children: List.generate(_labels.length * 2 - 1, (i) {
        if (i.isOdd) {
          return Expanded(
            child: Container(
              height: 2,
              color: i ~/ 2 < _currentStep
                  ? AppColors.primary
                  : Colors.grey.shade300,
            ),
          );
        }
        final idx = i ~/ 2;
        final done = idx < _currentStep;
        final active = idx == _currentStep;
        return _StepDot(
          label: _labels[idx],
          done: done,
          active: active,
        );
      }),
    );
  }
}

class _StepDot extends StatelessWidget {
  const _StepDot({
    required this.label,
    required this.done,
    required this.active,
  });

  final String label;
  final bool done;
  final bool active;

  @override
  Widget build(BuildContext context) {
    Color bg = done
        ? AppColors.primary
        : active
            ? AppColors.primaryLight
            : Colors.grey.shade300;

    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        CircleAvatar(
          radius: 12,
          backgroundColor: bg,
          child: done
              ? const Icon(Icons.check, size: 14, color: Colors.white)
              : const SizedBox.shrink(),
        ),
        const SizedBox(height: 4),
        Text(label,
            style: TextStyle(
              fontSize: 10,
              color: active ? AppColors.primary : Colors.grey,
              fontWeight: active ? FontWeight.bold : FontWeight.normal,
            )),
      ],
    );
  }
}
