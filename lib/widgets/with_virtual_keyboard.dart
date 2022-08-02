import 'dart:async';

import 'package:flutter/material.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/system_ui/virtual_keyboard/keyboard.dart';

class WithVirtualKeyboard extends StatefulWidget {
  final int viewId;
  final Widget child;

  // final BoxConstraints constraints;

  WithVirtualKeyboard({
    Key? key,
    required this.viewId,
    required this.child,
    // required this.constraints,
  }) : super(key: GlobalKey());

  @override
  State<WithVirtualKeyboard> createState() => _WithVirtualKeyboardState();
}

class _WithVirtualKeyboardState extends State<WithVirtualKeyboard> with SingleTickerProviderStateMixin {
  late var slideAnimationController = AnimationController(
    vsync: this,
    duration: const Duration(milliseconds: 300),
  );

  late var slideAnimation = CurvedAnimation(
    parent: slideAnimationController,
    curve: Curves.easeOutCubic,
    reverseCurve: Curves.easeInCubic,
  ).drive(
    Tween(
      begin: const Offset(0, 1),
      end: Offset.zero,
    ),
  );

  StreamSubscription? textInputEventsSubscription;

  @override
  void initState() {
    super.initState();
    () async {
      textInputEventsSubscription = PlatformApi.getTextInputEventsForViewId(widget.viewId).listen((event) {
        if (event is TextInputEnable) {
          slideAnimationController.forward();
        } else if (event is TextInputDisable) {
          slideAnimationController.reverse();
        } else if (event is TextInputCommit) {
          slideAnimationController.forward();
        }
      });
    }();
  }

  @override
  void dispose() {
    textInputEventsSubscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        widget.child,
        Align(
          alignment: Alignment.bottomCenter,
          child: SizedBox(
            width: 500,
            child: ClipRect(
              child: SlideTransition(
                position: slideAnimation,
                child: VirtualKeyboard(
                  onDismiss: () => slideAnimationController.reverse(),
                  onCharacter: (String char) => PlatformApi.insertText(widget.viewId, char),
                ),
              ),
            ),
          ),
        ),
      ],
    );
  }
}
