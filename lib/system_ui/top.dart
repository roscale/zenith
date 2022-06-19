import 'dart:math';

import 'package:flutter/material.dart';
import 'package:zenith/system_ui/quick_settings.dart';
import 'package:zenith/system_ui/status_bar.dart';

class Top extends StatefulWidget {
  const Top({Key? key}) : super(key: key);

  @override
  State<Top> createState() => _TopState();
}

class _TopState extends State<Top> with SingleTickerProviderStateMixin {
  bool quickSettingsVisible = false;

  var startDragHorizontalPosition = 0.0;

  late var controller = AnimationController(
    vsync: this,
    duration: const Duration(seconds: 1),
  );

  late var quickSettingsAnimation = controller.drive(
    Tween(
      begin: const Offset(0, -1),
      end: Offset.zero,
    ),
  );

  double verticalExpansionThreshold = 0.0;

  @override
  void dispose() {
    controller.dispose();
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
            if (controller.value <= 0.0) {
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
            controller.value += details.delta.dy / height;
            if (controller.value >= 1.0 && verticalExpansionThreshold == 0) {
              verticalExpansionThreshold = details.globalPosition.dy;
            }
          },
          onVerticalDragEnd: (details) {
            handleController(details.velocity.pixelsPerSecond.dy);
          },
        ),
        AnimatedBuilder(
          animation: controller,
          builder: (BuildContext context, Widget? child) {
            return IgnorePointer(
              ignoring: controller.value.abs() < 0.1,
              child: Container(
                color: Colors.black.withOpacity(controller.value / 2),
                child: child!,
              ),
            );
          },
          child: GestureDetector(
            onVerticalDragUpdate: (details) {
              controller.value += details.delta.dy / height;
            },
            onVerticalDragEnd: (details) {
              handleController(details.velocity.pixelsPerSecond.dy);
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
                child: const QuickSettings(),
                onVerticalDragUpdate: (details) {
                  controller.value += details.delta.dy / height;
                },
                onVerticalDragEnd: (details) {
                  // springDescription: SpringDescription.withDampingRatio(
                  //   mass: 1,
                  //   stiffness: 10,
                  // ),
                  handleController(details.velocity.pixelsPerSecond.dy);
                },
              ),
            ),
          ),
        ),
      ],
    );
  }

  void handleController(double velocity) {
    if (velocity.abs() > 300) {
      if (velocity > 0) {
        controller.fling(velocity: 1);
      } else {
        controller.fling(velocity: -1);
      }
    } else {
      if (controller.value >= 0.5) {
        controller.fling(velocity: 1);
      } else {
        controller.fling(velocity: -1);
      }
    }
  }
}
