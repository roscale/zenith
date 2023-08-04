import 'package:flutter/material.dart';
import 'package:flutter_portal/flutter_portal.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/desktop/task_bar.dart';
import 'package:zenith/ui/desktop/window_manager.dart';

class DesktopUi extends ConsumerStatefulWidget {
  const DesktopUi({Key? key}) : super(key: key);

  @override
  ConsumerState<DesktopUi> createState() => _DesktopUiState();
}

class _DesktopUiState extends ConsumerState<DesktopUi> {
  final backgroundFocusNode = FocusNode();

  @override
  Widget build(BuildContext context) {
    return Portal(
      child: Stack(
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
          Overlay(
            initialEntries: [
              OverlayEntry(
                builder: (_) => const Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Expanded(
                      child: WindowManager(),
                    ),
                    TaskBar(),
                  ],
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  @override
  void dispose() {
    backgroundFocusNode.dispose();
    super.dispose();
  }
}
