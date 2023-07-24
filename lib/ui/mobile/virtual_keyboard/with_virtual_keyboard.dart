import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/xdg_toplevel_state.dart';
import 'package:zenith/ui/mobile/state/task_switcher_state.dart';
import 'package:zenith/ui/mobile/state/virtual_keyboard_state.dart';
import 'package:zenith/ui/mobile/virtual_keyboard/animated_virtual_keyboard.dart';
import 'package:zenith/ui/mobile/virtual_keyboard/layouts.dart';

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
    return Consumer(
      builder: (_, WidgetRef ref, Widget? child) {
        if (widget.viewId != 0) {
          ref.watch(taskSwitcherStateProvider.select((v) => v.constraintsChanged));
          final constraints = taskSwitcherConstraints;
          final keyboardActivated = ref.watch(virtualKeyboardStateProvider(widget.viewId).select((v) => v.activated));
          final keyboardSize = ref.watch(virtualKeyboardStateProvider(widget.viewId).select((v) => v.keyboardSize));

          ref.read(xdgToplevelStatesProvider(widget.viewId).notifier)
            ..maximize(true)
            ..resize(
              constraints.maxWidth.toInt(),
              keyboardActivated && !keyboardSize.isEmpty
                  ? (constraints.maxHeight - keyboardSize.height).toInt()
                  : constraints.maxHeight.toInt(),
            );
        }
        return child!;
      },
      child: AnimatedVirtualKeyboard(
        key: key,
        id: widget.viewId,
        onDismiss: () => PlatformApi.hideKeyboard(widget.viewId),
        onCharacter: (String char) => PlatformApi.insertText(widget.viewId, char),
        onKeyCode: (KeyCode keyCode) => PlatformApi.emulateKeyCode(widget.viewId, keyCode.code),
        child: widget.child,
      ),
    );
  }
}
