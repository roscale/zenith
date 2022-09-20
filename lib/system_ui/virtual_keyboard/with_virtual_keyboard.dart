import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/system_ui/virtual_keyboard/layouts.dart';
import 'package:zenith/system_ui/virtual_keyboard/virtual_keyboard.dart';

final virtualKeyboardProvider = StateNotifierProvider.autoDispose
    .family<WithVirtualKeyboardNotifier, WithVirtualKeyboardProviderState, int>((ref, int viewId) {
  return WithVirtualKeyboardNotifier(const WithVirtualKeyboardProviderState.empty());
});

class WithVirtualKeyboardProviderState {
  const WithVirtualKeyboardProviderState({
    required this.activated,
    required this.dragging,
    required this.verticalDragOffset,
    required this.keyboardSize,
  });

  const WithVirtualKeyboardProviderState.empty()
      : this(
          activated: false,
          dragging: false,
          verticalDragOffset: 0,
          keyboardSize: Size.zero,
        );

  final bool activated;
  final bool dragging;
  final int verticalDragOffset;
  final Size keyboardSize;

  WithVirtualKeyboardProviderState copyWith({
    bool? activated,
    bool? dragging,
    int? verticalDragOffset,
    Size? keyboardSize,
  }) {
    return WithVirtualKeyboardProviderState(
      activated: activated ?? this.activated,
      dragging: dragging ?? this.dragging,
      verticalDragOffset: verticalDragOffset ?? this.verticalDragOffset,
      keyboardSize: keyboardSize ?? this.keyboardSize,
    );
  }
}

class WithVirtualKeyboardNotifier extends StateNotifier<WithVirtualKeyboardProviderState> {
  WithVirtualKeyboardNotifier(state) : super(state);

  set activated(bool value) => state = state.copyWith(activated: value);

  set keyboardSize(Size value) => state = state.copyWith(keyboardSize: value);

  void drag(int dy) {
    state = state.copyWith(
      dragging: true,
      verticalDragOffset: dy,
    );
  }

  void endDrag() {
    state = state.copyWith(
      dragging: false,
    );
  }
}

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
  )..addStatusListener((status) {
      final state = ref.read(virtualKeyboardProvider(widget.viewId));
      if (status == AnimationStatus.completed && !state.activated) {
        keyboardOverlay!.remove();
        keyboardOverlay = null;
      }
    });

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
    final notifier = ref.read(virtualKeyboardProvider(widget.viewId).notifier);
    final state = ref.read(virtualKeyboardProvider(widget.viewId));

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
      final notifier = ref.read(virtualKeyboardProvider(widget.viewId).notifier);
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
    ref.listen<bool>(virtualKeyboardProvider(widget.viewId).select((value) => value.activated), (previous, next) {
      next ? animateForward() : animateBackward();
    });

    return ClipRect(
      child: LayoutBuilder(
        builder: (BuildContext context, BoxConstraints constraints) {
          return Consumer(
            builder: (_, __, Widget? child) {
              final shown = ref.watch(virtualKeyboardProvider(widget.viewId).select((value) => value.activated));
              final keyboardSize =
                  ref.watch(virtualKeyboardProvider(widget.viewId).select((value) => value.keyboardSize));

              if (shown && keyboardSize.isEmpty) {
                SchedulerBinding.instance.addPostFrameCallback(_determineVirtualKeyboardSize);
              }

              PlatformApi.resizeWindow(
                widget.viewId,
                constraints.maxWidth.toInt(),
                shown && !keyboardSize.isEmpty
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
          );
        },
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
    ref.read(virtualKeyboardProvider(widget.viewId).notifier).keyboardSize = size;
  }

  OverlayEntry? newKeyboardOverlay() {
    return OverlayEntry(builder: (_) {
      return Align(
        alignment: Alignment.bottomCenter,
        child: ValueListenableBuilder(
          valueListenable: slideAnimation,
          builder: (_, Animation<Offset> animation, Widget? child) {
            return SlideTransition(
              position: animation,
              child: child!,
            );
          },
          child: RepaintBoundary(
            child: VirtualKeyboard(
              key: key,
              onDismiss: () => ref.read(virtualKeyboardProvider(widget.viewId).notifier).activated = false,
              onCharacter: (String char) => PlatformApi.insertText(widget.viewId, char),
              onKeyCode: (KeyCode keyCode) => PlatformApi.emulateKeyCode(widget.viewId, keyCode.code),
            ),
          ),
        ),
      );
    });
  }
}
