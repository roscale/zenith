import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/task_switcher_state.dart';
import 'package:zenith/state/virtual_keyboard_state.dart';
import 'package:zenith/system_ui/virtual_keyboard/layouts.dart';
import 'package:zenith/system_ui/virtual_keyboard/virtual_keyboard.dart';

class WithVirtualKeyboard extends ConsumerStatefulWidget {
  final int viewId;
  final Widget child;

  const WithVirtualKeyboard({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: key);

  @override
  ConsumerState<WithVirtualKeyboard> createState() => WithVirtualKeyboardState();
}

class WithVirtualKeyboardState extends ConsumerState<WithVirtualKeyboard> with SingleTickerProviderStateMixin {
  final key = GlobalKey();
  final overlayKey = GlobalKey<OverlayState>();

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

  late final keyboardWidget = Align(
    alignment: Alignment.bottomCenter,
    child: ValueListenableBuilder(
      valueListenable: slideAnimation,
      builder: (_, Animation<Offset> animation, Widget? child) {
        return SlideTransition(
          position: animation,
          child: child!,
        );
      },
      child: ProviderScope(
        overrides: [
          keyboardIdProvider.overrideWithValue(widget.viewId),
        ],
        child: RepaintBoundary(
          child: VirtualKeyboard(
            key: key,
            onDismiss: () => ref.read(virtualKeyboardStateProvider(widget.viewId).notifier).activated = false,
            onCharacter: (String char) => PlatformApi.insertText(widget.viewId, char),
            onKeyCode: (KeyCode keyCode) => PlatformApi.emulateKeyCode(widget.viewId, keyCode.code),
          ),
        ),
      ),
    ),
  );

  OverlayEntry? keyboardOverlay;

  void animateForward() {
    slideAnimation.value = animation.drive(
      Tween(
        begin: slideAnimation.value.value,
        end: Offset.zero,
      ),
    );
    slideAnimationController
      ..reset()
      ..forward();

    if (keyboardOverlay == null) {
      keyboardOverlay = newKeyboardOverlay();
      overlayKey.currentState!.insert(keyboardOverlay!);
    }
  }

  void animateBackward() {
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

  void dragKeyboard(double dy) {
    final notifier = ref.read(virtualKeyboardStateProvider(widget.viewId).notifier);
    final state = ref.read(virtualKeyboardStateProvider(widget.viewId));

    if (state.activated) {
      slideAnimationController.value += dy / state.keyboardSize.height;
      return;
    }
    notifier.activated = true;
    slideAnimation.value = slideAnimationController.drive(
      Tween(
        begin: slideAnimation.value.value,
        end: Offset.zero,
      ),
    );
    if (keyboardOverlay == null) {
      keyboardOverlay = newKeyboardOverlay();
      overlayKey.currentState!.insert(keyboardOverlay!);
    }
  }

  StreamSubscription? textInputEventsSubscription;

  @override
  void initState() {
    super.initState();
    textInputEventsSubscription = PlatformApi.getTextInputEventsForViewId(widget.viewId).listen((event) {
      final notifier = ref.read(virtualKeyboardStateProvider(widget.viewId).notifier);
      if (event is TextInputEnable) {
        notifier.activated = true;
      } else if (event is TextInputDisable) {
        notifier.activated = false;
      } else if (event is TextInputCommit) {
        notifier.activated = true;
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
    ref.listen<bool>(virtualKeyboardStateProvider(widget.viewId).select((value) => value.activated), (previous, next) {
      next ? animateForward() : animateBackward();
    });

    return ClipRect(
      child: Consumer(
        builder: (_, WidgetRef ref, Widget? child) {
          final constraints = ref.watch(taskSwitcherStateProvider.select((v) => v.constraints));
          final keyboardActivated = ref.watch(virtualKeyboardStateProvider(widget.viewId).select((v) => v.activated));
          final keyboardSize = ref.watch(virtualKeyboardStateProvider(widget.viewId).select((v) => v.keyboardSize));

          if (keyboardActivated && keyboardSize.isEmpty) {
            SchedulerBinding.instance.addPostFrameCallback(_determineVirtualKeyboardSize);
          }

          PlatformApi.resizeWindow(
            widget.viewId,
            constraints.maxWidth.toInt(),
            keyboardActivated && !keyboardSize.isEmpty
                ? (constraints.maxHeight - keyboardSize.height).toInt()
                : constraints.maxHeight.toInt(),
          );

          return child!;
        },
        child: Overlay(
          key: overlayKey,
          initialEntries: [
            OverlayEntry(builder: (_) => widget.child),
          ],
        ),
      ),
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
    ref.read(virtualKeyboardStateProvider(widget.viewId).notifier).keyboardSize = size;
  }

  OverlayEntry? newKeyboardOverlay() {
    return OverlayEntry(
      builder: (_) => keyboardWidget,
    );
  }
}
