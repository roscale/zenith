import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/desktop/task_bar.dart';
import 'package:zenith/ui/desktop/window_manager.dart';
import 'package:zenith/ui/mobile/quick_settings/status_bar_with_quick_settings.dart';

class DesktopUi extends ConsumerStatefulWidget {
  const DesktopUi({Key? key}) : super(key: key);

  @override
  ConsumerState<DesktopUi> createState() => _DesktopUiState();
}

class _DesktopUiState extends ConsumerState<DesktopUi> {
  final backgroundFocusNode = FocusNode();

  @override
  Widget build(BuildContext context) {
    return Stack(
      clipBehavior: Clip.none,
      fit: StackFit.expand,
      children: [
        Focus(
          focusNode: backgroundFocusNode,
          child: GestureDetector(
            onTapDown: (_) {
              backgroundFocusNode.requestFocus();
            },
            child: Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
          ),
        ),
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

  @override
  void dispose() {
    backgroundFocusNode.dispose();
    super.dispose();
  }
}
