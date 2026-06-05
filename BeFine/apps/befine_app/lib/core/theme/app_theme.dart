import 'package:flutter/material.dart';

abstract final class AppColors {
  static const primary = Color(0xFF1565C0);
  static const primaryLight = Color(0xFF5E92F3);
  static const primaryDark = Color(0xFF003C8F);
  static const secondary = Color(0xFF00ACC1);
  static const background = Color(0xFFF5F7FA);
  static const surface = Colors.white;
  static const error = Color(0xFFD32F2F);
  static const scoreA = Color(0xFF2E7D32);
  static const scoreB = Color(0xFF1565C0);
  static const scoreC = Color(0xFFEF6C00);
  static const scoreF = Color(0xFFD32F2F);
  static const onPrimary = Colors.white;
  static const onBackground = Color(0xFF1A1A2E);
}

abstract final class AppTheme {
  static ThemeData get light => ThemeData(
        useMaterial3: true,
        colorScheme: ColorScheme.fromSeed(
          seedColor: AppColors.primary,
          brightness: Brightness.light,
          surface: AppColors.surface,
        ),
        scaffoldBackgroundColor: AppColors.background,
        appBarTheme: const AppBarTheme(
          backgroundColor: AppColors.primary,
          foregroundColor: AppColors.onPrimary,
          elevation: 0,
          centerTitle: true,
        ),
        cardTheme: CardThemeData(
          elevation: 2,
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(16),
          ),
        ),
        elevatedButtonTheme: ElevatedButtonThemeData(
          style: ElevatedButton.styleFrom(
            backgroundColor: AppColors.primary,
            foregroundColor: AppColors.onPrimary,
            padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 14),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(12),
            ),
          ),
        ),
        textTheme: const TextTheme(
          displaySmall: TextStyle(
              fontSize: 32,
              fontWeight: FontWeight.bold,
              color: AppColors.onBackground),
          headlineMedium: TextStyle(
              fontSize: 24,
              fontWeight: FontWeight.w600,
              color: AppColors.onBackground),
          titleLarge: TextStyle(
              fontSize: 18,
              fontWeight: FontWeight.w600,
              color: AppColors.onBackground),
          bodyLarge:
              TextStyle(fontSize: 16, color: AppColors.onBackground),
          bodyMedium:
              TextStyle(fontSize: 14, color: AppColors.onBackground),
        ),
      );
}
