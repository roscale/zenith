import 'dart:math';

import 'package:flutter/material.dart';
import 'package:zenith/system_ui/quick_settings/quick_settings.dart';
import 'package:zenith/system_ui/quick_settings/status_bar.dart';

class StatusBarWithQuickSettings extends StatefulWidget {
  const StatusBarWithQuickSettings({Key? key}) : super(key: key);

  @override
  State<StatusBarWithQuickSettings> createState() => _StatusBarWithQuickSettingsState();
}

class _StatusBarWithQuickSettingsState extends State<StatusBarWithQuickSettings> with SingleTickerProviderStateMixin {
  bool quickSettingsVisible = false;

  var startDragHorizontalPosition = 0.0;

  late var quickSettingsController = AnimationController(
    vsync: this,
    duration: const Duration(seconds: 1),
  );

  late var quickSettingsAnimation = quickSettingsController.drive(
    Tween(
      begin: const Offset(0, -1),
      end: Offset.zero,
    ),
  );

  // Fading is disabled while dragging the brightness slider.
  final disableFading = ValueNotifier(false);

  double verticalExpansionThreshold = 0.0;

  @override
  void dispose() {
    quickSettingsController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final width = min(MediaQuery.of(context).size.width, 600.0);
    const height = 300.0;

    return Stack(
      children: [
        StatusBar(
          onVerticalDragStart: (details) {
            verticalExpansionThreshold = 0;
            if (quickSettingsController.value <= 0.0) {
              setState(() {
                startDragHorizontalPosition = details.globalPosition.dx;
              });
            }
          },
          onVerticalDragUpdate: (details) {
            if (details.delta.dy < 0 &&
                details.globalPosition.dy > verticalExpansionThreshold &&
                verticalExpansionThreshold != 0) {
              return;
            }
            quickSettingsController.value += details.delta.dy / height;
            if (quickSettingsController.value >= 1.0 && verticalExpansionThreshold == 0) {
              verticalExpansionThreshold = details.globalPosition.dy;
            }
          },
          onVerticalDragEnd: (details) {
            startSlideAnimation(details.velocity.pixelsPerSecond.dy);
          },
        ),
        ValueListenableBuilder(
          valueListenable: quickSettingsController,
          builder: (BuildContext context, double animationValue, Widget? child) {
            return IgnorePointer(
              ignoring: animationValue.abs() < 0.1,
              child: AnimatedBuilder(
                animation: disableFading,
                builder: (_, __) {
                  return AnimatedOpacity(
                    opacity: disableFading.value ? 0 : 1,
                    duration: const Duration(milliseconds: 200),
                    curve: Curves.easeInOut,
                    child: Container(
                      color: Colors.black.withOpacity(animationValue / 2),
                      child: child!,
                    ),
                  );
                },
              ),
            );
          },
          child: GestureDetector(
            onVerticalDragUpdate: (details) {
              quickSettingsController.value += details.delta.dy / height;
            },
            onVerticalDragEnd: (details) {
              startSlideAnimation(details.velocity.pixelsPerSecond.dy);
            },
          ),
        ),
        Positioned(
          left: (startDragHorizontalPosition - width / 2).clamp(0, MediaQuery.of(context).size.width - width),
          child: SlideTransition(
            position: quickSettingsAnimation,
            child: SizedBox(
              width: width,
              height: height,
              child: GestureDetector(
                child: QuickSettings(
                  onChangeBrightnessStart: () => disableFading.value = true,
                  onChangeBrightnessEnd: () => disableFading.value = false,
                ),
                onVerticalDragUpdate: (details) {
                  quickSettingsController.value += details.delta.dy / height;
                },
                onVerticalDragEnd: (details) {
                  startSlideAnimation(details.velocity.pixelsPerSecond.dy);
                },
              ),
            ),
          ),
        ),
      ],
    );
  }

  void startSlideAnimation(double velocity) {
    if (velocity.abs() > 300) {
      // Flick.
      if (velocity > 0) {
        quickSettingsController.fling(velocity: 1);
      } else {
        quickSettingsController.fling(velocity: -1);
      }
    } else {
      if (quickSettingsController.value >= 0.5) {
        // Half way down.
        quickSettingsController.fling(velocity: 1);
      } else {
        quickSettingsController.fling(velocity: -1);
      }
    }
  }
}
