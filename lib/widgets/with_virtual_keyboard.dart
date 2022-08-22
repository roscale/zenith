import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/system_ui/virtual_keyboard/key_codes.dart';
import 'package:zenith/system_ui/virtual_keyboard/keyboard.dart';
import 'package:zenith/util/multi_value_listenable_builder.dart';

class WithVirtualKeyboard extends StatefulWidget {
  final int viewId;
  final Widget child;

  const WithVirtualKeyboard({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: key);

  @override
  State<WithVirtualKeyboard> createState() => _WithVirtualKeyboardState();
}

class _WithVirtualKeyboardState extends State<WithVirtualKeyboard> with SingleTickerProviderStateMixin {
  final shown = ValueNotifier(false);

  final key = GlobalKey();
  var keyboardSize = ValueNotifier(Size.zero);

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
    if (shown.value) {
      return;
    }
    shown.value = true;
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
    if (!shown.value) {
      return;
    }
    shown.value = false;
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
    SchedulerBinding.instance.addPostFrameCallback(_determineVirtualKeyboardSize);

    return LayoutBuilder(
      builder: (BuildContext context, BoxConstraints constraints) {
        return ValueListenableBuilder2(
          first: shown,
          second: keyboardSize,
          builder: (_, bool shown, Size keyboardSize, Widget? child) {
            PlatformApi.resizeWindow(
              widget.viewId,
              constraints.maxWidth.toInt(),
              shown && keyboardSize != Size.zero
                  ? (constraints.maxHeight - keyboardSize.height).toInt()
                  : constraints.maxHeight.toInt(),
            );

            return Stack(
              children: [
                widget.child,
                Align(
                  alignment: Alignment.bottomCenter,
                  child: ValueListenableBuilder(
                    valueListenable: slideAnimation,
                    builder: (_, Animation<Offset> animation, Widget? child) {
                      return SlideTransition(
                        position: animation,
                        child: child!,
                      );
                    },
                    child: VirtualKeyboard(
                      key: key,
                      onDismiss: () => animateBackward(),
                      onCharacter: (String char) => PlatformApi.insertText(widget.viewId, char),
                      onKeyCode: (KeyCode keyCode) => PlatformApi.emulateKeyCode(widget.viewId, keyCode.code),
                    ),
                  ),
                ),
              ],
            );
          },
        );
      },
    );
  }

  void _determineVirtualKeyboardSize(Duration timestamp) {
    var context = key.currentContext;
    if (context == null) {
      return;
    }
    final size = context.size;
    if (size == null) {
      return;
    }
    keyboardSize.value = size;
  }
}
