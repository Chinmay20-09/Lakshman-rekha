import 'package:flutter/material.dart';
import '../models/app_state.dart';
import '../widgets/alert_banner.dart';
import 'home_screen.dart';
import 'alerts_screen.dart';
import 'contacts_screen.dart';
import 'settings_screen.dart';
import 'sos_active_screen.dart';

class MainLayout extends StatefulWidget {
  const MainLayout({super.key});

  @override
  State<MainLayout> createState() => _MainLayoutState();
}

class _MainLayoutState extends State<MainLayout> {
  int _currentIndex = 0;
  bool _sosPushed = false;

  final List<Widget> _pages = const [
    HomeScreen(),
    AlertsScreen(),
    ContactsScreen(),
    SettingsScreen(),
  ];

  @override
  void initState() {
    super.initState();
    AppState().addListener(_onStateChange);
  }

  void _onStateChange() {
    if (!mounted) return;
    final state = AppState();
    if (state.currentAlertLevel == AlertLevel.sos && !_sosPushed) {
      _sosPushed = true;
      // Use a post frame callback to ensure navigation happens outside of build
      WidgetsBinding.instance.addPostFrameCallback((_) {
        if (!mounted) return;
        Navigator.of(context)
            .push(
              MaterialPageRoute(
                fullscreenDialog: true,
                builder: (_) => const SosActiveScreen(),
              ),
            )
            .then((_) {
              // reset flag when screen is dismissed
              _sosPushed = false;
            });
      });
    }
  }

  @override
  void dispose() {
    AppState().removeListener(_onStateChange);
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final state = AppState();
    return Scaffold(
      body: Stack(
        clipBehavior: Clip.none,
        children: [
          IndexedStack(index: _currentIndex, children: _pages),
          // Overlay alert banner that slides from top when active
          AlertBanner(
            visible: state.currentAlertLevel == AlertLevel.alert,
            message: state.alertMessage.isNotEmpty
                ? state.alertMessage
                : 'Emergency alert',
            onResolve: () => AppState().resolveAlerts(),
          ),
        ],
      ),
      bottomNavigationBar: NavigationBar(
        height: 72,
        selectedIndex: _currentIndex,
        onDestinationSelected: (index) => setState(() => _currentIndex = index),
        destinations: const [
          NavigationDestination(
            icon: Icon(Icons.home_outlined),
            selectedIcon: Icon(Icons.home),
            label: 'Home',
          ),
          NavigationDestination(
            icon: Icon(Icons.notification_important_outlined),
            selectedIcon: Icon(Icons.notification_important),
            label: 'Alerts',
          ),
          NavigationDestination(
            icon: Icon(Icons.person_outline),
            selectedIcon: Icon(Icons.person),
            label: 'Contacts',
          ),
          NavigationDestination(
            icon: Icon(Icons.settings_outlined),
            selectedIcon: Icon(Icons.settings),
            label: 'Settings',
          ),
        ],
      ),
    );
  }
}
