import 'package:flutter/material.dart';
import 'package:zenith/system_ui/app_drawer.dart';
import 'package:zenith/system_ui/invisible_drawer_bar.dart';

class Bottom extends StatefulWidget {
  const Bottom({Key? key}) : super(key: key);

  @override
  State<Bottom> createState() => _BottomState();
}

class _BottomState extends State<Bottom> with SingleTickerProviderStateMixin {
  late var controller = AnimationController(
    vsync: this,
    duration: const Duration(seconds: 1),
  );

  late var appDrawerAnimation = controller.drive(
    Tween(
      begin: const Offset(0, 1),
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
    return Stack(
      alignment: AlignmentDirectional.bottomStart,
      children: [
        InvisibleDrawerBar(
          onVerticalDragStart: (details) {
            verticalExpansionThreshold = 0;
          },
          onVerticalDragUpdate: (details) {
            if (details.delta.dy > 0 &&
                details.globalPosition.dy < verticalExpansionThreshold &&
                verticalExpansionThreshold != 0) {
              return;
            }
            controller.value -= details.delta.dy / MediaQuery.of(context).size.height;
            if (controller.value >= 1.0 && verticalExpansionThreshold == 0) {
              verticalExpansionThreshold = details.globalPosition.dy;
            }
          },
          onVerticalDragEnd: (details) {
            handleController(details.velocity.pixelsPerSecond.dy);
          },
        ),
        Positioned(
          bottom: 0,
          child: SlideTransition(
            position: appDrawerAnimation,
            child: SizedBox(
              width: MediaQuery.of(context).size.width,
              height: MediaQuery.of(context).size.height,
              child: GestureDetector(
                child: const AppDrawer(),
                onVerticalDragUpdate: (details) {
                  controller.value -= details.delta.dy / MediaQuery.of(context).size.height;
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
      if (velocity < 0) {
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
