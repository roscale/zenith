import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
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
        PlatformApi.activateWindow(viewId);
        ref.read(windowStackProvider.notifier).raise(viewId);
      },
      child: child,
    );
  }
}
