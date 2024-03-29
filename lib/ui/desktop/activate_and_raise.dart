import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/xdg_toplevel_state.dart';
import 'package:zenith/ui/desktop/state/window_stack_provider.dart';

class ActivateAndRaise extends ConsumerWidget {
  final int viewId;
  final Widget child;

  const ActivateAndRaise({
    super.key,
    required this.viewId,
    required this.child,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Listener(
      onPointerDown: (_) {
        ref.read(xdgToplevelStatesProvider(viewId)).focusNode.requestFocus();
        ref.read(windowStackProvider.notifier).raise(viewId);
      },
      child: child,
    );
  }
}
