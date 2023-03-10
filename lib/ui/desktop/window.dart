import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/ui/common/xdg_toplevel_surface.dart';
import 'package:zenith/ui/desktop/manual_pan_gesture_recognizer.dart';
import 'package:zenith/ui/desktop/server_side_decorations/server_side_decorations.dart';
import 'package:zenith/ui/desktop/state/resizing_state_notifier_provider.dart';
import 'package:zenith/ui/desktop/state/window_move_state_notifier_provider.dart';
import 'package:zenith/ui/desktop/window_manager.dart';

final windowPositionStateProvider = StateProvider.family<Offset, int>((ref, int viewId) => Offset.zero);

final windowWidget = StateProvider.family<Window, int>((ref, int viewId) {
  return Window(
    key: GlobalKey(),
    viewId: viewId,
  );
});

class Window extends ConsumerStatefulWidget {
  final int viewId;

  const Window({
    super.key,
    required this.viewId,
  });

  @override
  ConsumerState<Window> createState() => _WindowState();
}

class _WindowState extends ConsumerState<Window> {
  ProviderSubscription? interactiveMoveSubscription;

  @override
  Widget build(BuildContext context) {
    ref.listen(zenithXdgSurfaceStateProvider(widget.viewId).select((v) => v.visibleBounds),
        (Rect? previous, Rect? next) {
      if (previous != null && next != null) {
        Offset offset = ref
            .read(resizingStateNotifierProvider(widget.viewId).notifier)
            .computeWindowOffset(previous.size, next.size);
        ref.read(windowPositionStateProvider(widget.viewId).notifier).update((state) => state + offset);
      }
    });

    ref.listen(windowMoveStateNotifierProvider(widget.viewId).select((v) => v.movedPosition), (_, Offset? position) {
      if (position != null) {
        ref.read(windowPositionStateProvider(widget.viewId).notifier).state = Offset(
          position.dx.roundToDouble(),
          position.dy.roundToDouble(),
        );
      }
    });

    return Consumer(
      builder: (BuildContext context, WidgetRef ref, Widget? child) {
        final offset = ref.watch(windowPositionStateProvider(widget.viewId));
        return Positioned(
          left: offset.dx,
          top: offset.dy,
          child: child!,
        );
      },
      child: ServerSideDecorations(
        viewId: widget.viewId,
        child: RawGestureDetector(
          gestures: <Type, GestureRecognizerFactory>{
            ManualPanGestureRecognizer: GestureRecognizerFactoryWithHandlers<ManualPanGestureRecognizer>(
              () => ManualPanGestureRecognizer(),
              (ManualPanGestureRecognizer instance) {
                instance.dragStartBehavior = DragStartBehavior.down;
                instance.onDown = (_) {
                  PlatformApi.activateWindow(widget.viewId);
                  ref.read(windowStackNotifierProvider.notifier).raise(widget.viewId);

                  ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).startPotentialMove();

                  interactiveMoveSubscription = ref.listenManual(
                      zenithXdgToplevelStateProvider(widget.viewId).select((v) => v.interactiveMoveRequested), (_, __) {
                    ref
                        .read(windowMoveStateNotifierProvider(widget.viewId).notifier)
                        .startMove(ref.read(windowPositionStateProvider(widget.viewId)));

                    instance.claimVictory();
                  });
                };
                instance.onUpdate = (DragUpdateDetails event) {
                  ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).move(event.delta);
                };
                instance.onEnd = (_) {
                  ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).endMove();
                  interactiveMoveSubscription?.close();
                };
                instance.onCancel = () {
                  ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).cancelMove();
                  interactiveMoveSubscription?.close();
                };
              },
            ),
          },
          child: Consumer(
            builder: (BuildContext context, WidgetRef ref, Widget? child) {
              return ref.watch(xdgToplevelSurfaceWidget(widget.viewId));
            },
          ),
        ),
      ),
    );
  }
}
