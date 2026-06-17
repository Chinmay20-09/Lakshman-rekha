import 'package:flutter/material.dart';
import 'theme/app_theme.dart';
import 'screens/splash_screen.dart';
import 'models/app_state.dart';

void main() {
  runApp(const LakshmanRekhaApp());
}

class LakshmanRekhaApp extends StatefulWidget {
  const LakshmanRekhaApp({super.key});

  @override
  State<LakshmanRekhaApp> createState() => _LakshmanRekhaAppState();
}

class _LakshmanRekhaAppState extends State<LakshmanRekhaApp> {
  @override
  void initState() {
    super.initState();
    // Listen to theme changes from the global AppState singleton
    AppState().addListener(_onThemeChange);
  }

  void _onThemeChange() {
    // Only rebuild if the dark mode setting actually changes the theme we want to display
    if (mounted) setState(() {});
  }

  @override
  void dispose() {
    AppState().removeListener(_onThemeChange);
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final isDarkMode = AppState().isDarkMode;
    return MaterialApp(
      title: 'Lakshman Rekha',
      debugShowCheckedModeBanner: false,
      theme: AppTheme.lightTheme,
      darkTheme: AppTheme.darkTheme,
      themeMode: isDarkMode ? ThemeMode.dark : ThemeMode.light,
      home: const SplashScreen(),
    );
  }
}
