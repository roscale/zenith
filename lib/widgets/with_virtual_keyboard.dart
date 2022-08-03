import 'dart:async';

import 'package:flutter/material.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/system_ui/virtual_keyboard/key_codes.dart';
import 'package:zenith/system_ui/virtual_keyboard/keyboard.dart';

class WithVirtualKeyboard extends StatefulWidget {
  final int viewId;
  final Widget child;

  WithVirtualKeyboard({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: GlobalKey());

  @override
  State<WithVirtualKeyboard> createState() => _WithVirtualKeyboardState();
}

class _WithVirtualKeyboardState extends State<WithVirtualKeyboard> with SingleTickerProviderStateMixin {
  bool shown = false;

  late var slideAnimationController = AnimationController(
    vsync: this,
    duration: const Duration(milliseconds: 300),
  );

  late var animation = CurvedAnimation(
    parent: slideAnimationController,
    curve: Curves.easeOutCubic,
  );

  late ValueNotifier<Animation<Offset>> slideAnimation = ValueNotifier(
    animation.drive(
      Tween(
        begin: const Offset(0, 1),
        end: Offset.zero,
      ),
    ),
  );

  void animateForward() {
    if (shown) {
      return;
    }
    shown = true;
    slideAnimation.value = animation.drive(
      Tween(
        begin: slideAnimation.value.value,
        end: Offset.zero,
      ),
    );
    slideAnimationController
      ..reset()
      ..forward();
  }

  void animateBackward() {
    if (!shown) {
      return;
    }
    shown = false;
    slideAnimation.value = animation.drive(
      Tween(
        begin: slideAnimation.value.value,
        end: const Offset(0, 1),
      ),
    );
    slideAnimationController
      ..reset()
      ..forward();
  }

  StreamSubscription? textInputEventsSubscription;

  @override
  void initState() {
    super.initState();
    textInputEventsSubscription = PlatformApi.getTextInputEventsForViewId(widget.viewId).listen((event) {
      if (event is TextInputEnable) {
        animateForward();
      } else if (event is TextInputDisable) {
        animateBackward();
      } else if (event is TextInputCommit) {
        animateForward();
      }
    });
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
              child: ValueListenableBuilder(
                valueListenable: slideAnimation,
                builder: (_, Animation<Offset> animation, Widget? child) {
                  return SlideTransition(
                    position: animation,
                    child: child!,
                  );
                },
                child: VirtualKeyboard(
                  onDismiss: () => animateBackward(),
                  onCharacter: (String char) => PlatformApi.insertText(widget.viewId, char),
                  onKeyCode: (KeyCode keyCode) => PlatformApi.emulateKeyCode(widget.viewId, keyCode.code),
                ),
              ),
            ),
          ),
        ),
      ],
    );
  }
}
