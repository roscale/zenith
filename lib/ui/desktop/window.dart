import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/xdg_surface_state.dart';
import 'package:zenith/ui/common/state/xdg_toplevel_state.dart';
import 'package:zenith/ui/desktop/decorations/with_decorations.dart';
import 'package:zenith/ui/desktop/interactive_move_and_resize_listener.dart';
import 'package:zenith/ui/desktop/state/window_move_provider.dart';
import 'package:zenith/ui/desktop/state/window_resize_provider.dart';

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
    ref.listen(xdgSurfaceStatesProvider(viewId).select((v) => v.visibleBounds), (Rect? previous, Rect? next) {
      if (previous != null && next != null) {
        Offset offset = ref.read(windowResizeProvider(viewId).notifier).computeWindowOffset(previous.size, next.size);
        ref.read(windowPositionStateProvider(viewId).notifier).update((state) => state + offset);
      }
    });

    ref.listen(windowMoveProvider(viewId).select((v) => v.movedPosition), (_, Offset? position) {
      if (position != null) {
        ref.read(windowPositionStateProvider(viewId).notifier).state = Offset(
          position.dx.roundToDouble(),
          position.dy.roundToDouble(),
        );
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
      child: WithDecorations(
        viewId: viewId,
        child: InteractiveMoveAndResizeListener(
          viewId: viewId,
          child: Consumer(
            builder: (BuildContext context, WidgetRef ref, Widget? child) {
              return ref.watch(xdgToplevelSurfaceWidgetProvider(viewId));
            },
          ),
        ),
      ),
    );
  }
}
