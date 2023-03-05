import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/zenith_surface_state.dart';
import 'package:zenith/ui/common/xdg_toplevel_surface.dart';
import 'package:zenith/ui/desktop/server_side_decorations/server_side_decorations.dart';
import 'package:zenith/ui/desktop/state/resizing_state_notifier_provider.dart';
import 'package:zenith/ui/desktop/window_manager.dart';

final windowPositionStateProvider = StateProvider.family<Offset, int>((ref, int viewId) => Offset.zero);

final windowWidget = StateProvider.family<Window, int>((ref, int viewId) {
  return Window(
    key: GlobalKey(),
    viewId: viewId,
  );
});

class Window extends ConsumerWidget {
  final int viewId;

  const Window({
    super.key,
    required this.viewId,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    ref.listen(zenithSurfaceStateProvider(viewId).select((v) => v.surfaceSize), (Size? previous, Size? next) {
      if (previous != null && next != null) {
        Offset offset = ref.read(resizingStateNotifierProvider(viewId).notifier).computeWindowOffset(previous, next);
        ref.read(windowPositionStateProvider(viewId).notifier).update((state) => state + offset);
      }
    });

    return Consumer(
      builder: (BuildContext context, WidgetRef ref, Widget? child) {
        final offset = ref.watch(windowPositionStateProvider(viewId));
        return Positioned(
          left: offset.dx,
          top: offset.dy,
          child: child!,
        );
      },
      child: Listener(
        onPointerDown: (_) {
          PlatformApi.activateWindow(viewId);
          ref.read(windowStackNotifierProvider.notifier).raise(viewId);
        },
        child: Consumer(
          builder: (BuildContext context, WidgetRef ref, Widget? child) {
            return ServerSideDecorations(
              viewId: viewId,
              child: ref.watch(xdgToplevelSurfaceWidget(viewId)),
            );
          },
        ),
      ),
    );
  }
}
