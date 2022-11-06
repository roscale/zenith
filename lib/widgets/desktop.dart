import 'package:flutter/material.dart';
import 'package:zenith/system_ui/quick_settings/status_bar_with_quick_settings.dart';
import 'package:zenith/system_ui/task_switcher/task_switcher.dart';

class Desktop extends StatelessWidget {
  const Desktop({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Stack(
      fit: StackFit.expand,
      children: [
        Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
        SafeArea(
          child: Overlay(
            initialEntries: [
              OverlayEntry(
                builder: (_) => const TaskSwitcher(
                  spacing: 20,
                ),
              ),
            ],
          ),
        ),
        const StatusBarWithQuickSettings(),
      ],
    );
  }
}
