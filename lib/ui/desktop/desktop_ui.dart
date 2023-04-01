import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/desktop/task_bar.dart';
import 'package:zenith/ui/desktop/window_manager.dart';
import 'package:zenith/ui/mobile/quick_settings/status_bar_with_quick_settings.dart';

class DesktopUi extends ConsumerWidget {
  const DesktopUi({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Stack(
      clipBehavior: Clip.none,
      fit: StackFit.expand,
      children: [
        Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
        SafeArea(
          child: Overlay(
            initialEntries: [
              OverlayEntry(
                builder: (_) => Column(
                  mainAxisSize: MainAxisSize.min,
                  children: const [
                    Expanded(
                      child: WindowManager(),
                    ),
                    TaskBar(),
                  ],
                ),
              ),
            ],
          ),
        ),
        const RepaintBoundary(
          child: StatusBarWithQuickSettings(),
        ),
      ],
    );
  }
}
