import 'package:flutter/material.dart';

class AlertBanner extends StatelessWidget {
  final bool visible;
  final String message;
  final VoidCallback onResolve;

  const AlertBanner({
    super.key,
    required this.visible,
    required this.message,
    required this.onResolve,
  });

  @override
  Widget build(BuildContext context) {
    const double height = 76.0;
    final topInset = MediaQuery.of(context).padding.top;
    final hiddenTop = -height - topInset;

    return AnimatedPositioned(
      duration: const Duration(milliseconds: 350),
      curve: Curves.easeOutCubic,
      top: visible ? 0 : hiddenTop,
      left: 0,
      right: 0,
      height: height + topInset,
      child: SafeArea(
        bottom: false,
        child: Material(
          elevation: 6,
          color: Colors.red.shade700,
          child: Container(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
            child: Row(
              children: [
                const Icon(Icons.warning_amber_rounded, color: Colors.white),
                const SizedBox(width: 12),
                Expanded(
                  child: Text(
                    message,
                    style: const TextStyle(color: Colors.white, fontWeight: FontWeight.w600),
                  ),
                ),
                TextButton(
                  onPressed: onResolve,
                  style: TextButton.styleFrom(foregroundColor: Colors.white),
                  child: const Text('RESOLVE'),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
