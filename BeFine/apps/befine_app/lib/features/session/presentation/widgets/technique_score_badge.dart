import 'package:befine_ble_sdk/befine_ble_sdk.dart';
import 'package:flutter/material.dart';
import '../../../../core/theme/app_theme.dart';

class TechniqueScoreBadge extends StatelessWidget {
  const TechniqueScoreBadge({
    super.key,
    required this.score,
    this.size = 48,
  });

  final ScoreGrade score;
  final double size;

  Color get _color => switch (score) {
        ScoreGrade.a => AppColors.scoreA,
        ScoreGrade.b => AppColors.scoreB,
        ScoreGrade.c => AppColors.scoreC,
        ScoreGrade.f => AppColors.scoreF,
      };

  @override
  Widget build(BuildContext context) {
    return Container(
      width: size,
      height: size,
      decoration: BoxDecoration(
        color: _color,
        shape: BoxShape.circle,
        boxShadow: [
          BoxShadow(
            color: _color.withOpacity(0.35),
            blurRadius: 12,
            offset: const Offset(0, 4),
          ),
        ],
      ),
      alignment: Alignment.center,
      child: Text(
        score.label,
        style: TextStyle(
          color: Colors.white,
          fontSize: size * 0.45,
          fontWeight: FontWeight.bold,
        ),
      ),
    );
  }
}
