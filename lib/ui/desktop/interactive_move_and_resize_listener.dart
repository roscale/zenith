import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/ui/desktop/manual_pan_gesture_recognizer.dart';
import 'package:zenith/ui/desktop/state/resizing_state_notifier_provider.dart';
import 'package:zenith/ui/desktop/state/window_move_state_notifier_provider.dart';
import 'package:zenith/ui/desktop/window.dart';
import 'package:zenith/ui/desktop/window_manager.dart';

class InteractiveMoveAndResizeListener extends ConsumerStatefulWidget {
  final int viewId;
  final Widget child;

  const InteractiveMoveAndResizeListener({
    super.key,
    required this.viewId,
    required this.child,
  });

  @override
  ConsumerState<InteractiveMoveAndResizeListener> createState() => _InteractiveMoveAndResizeListenerState();
}

class _InteractiveMoveAndResizeListenerState extends ConsumerState<InteractiveMoveAndResizeListener> {
  ProviderSubscription? interactiveMoveSubscription;
  ProviderSubscription? interactiveResizeSubscription;

  @override
  Widget build(BuildContext context) {
    return RawGestureDetector(
      gestures: <Type, GestureRecognizerFactory>{
        ManualPanGestureRecognizer: GestureRecognizerFactoryWithHandlers<ManualPanGestureRecognizer>(
          () => ManualPanGestureRecognizer(),
          (ManualPanGestureRecognizer instance) {
            instance.dragStartBehavior = DragStartBehavior.down;
            instance.onDown = (_) {
              PlatformApi.activateWindow(widget.viewId);
              ref.read(windowStackNotifierProvider.notifier).raise(widget.viewId);

              ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).startPotentialMove();
              ref.read(resizingStateNotifierProvider(widget.viewId).notifier).startPotentialResize();

              interactiveMoveSubscription = ref.listenManual(
                  zenithXdgToplevelStateProvider(widget.viewId).select((v) => v.interactiveMoveRequested), (_, __) {
                ref
                    .read(windowMoveStateNotifierProvider(widget.viewId).notifier)
                    .startMove(ref.read(windowPositionStateProvider(widget.viewId)));

                // We can start moving the window.
                // Cancel all other GestureDetectors in the arena.
                instance.claimVictory();
              });

              interactiveResizeSubscription = ref.listenManual<ResizeEdgeObject>(
                  zenithXdgToplevelStateProvider(widget.viewId).select((v) => v.interactiveResizeRequested),
                  (_, ResizeEdgeObject? resizeEdge) {
                if (resizeEdge == null) {
                  return;
                }

                Size size = ref.read(zenithXdgSurfaceStateProvider(widget.viewId)).visibleBounds.size;
                ref.read(resizingStateNotifierProvider(widget.viewId).notifier).startResize(resizeEdge.edge, size);

                // We can start resizing the window.
                // Cancel all other GestureDetectors in the arena.
                instance.claimVictory();
              });
            };
            instance.onUpdate = (DragUpdateDetails event) {
              ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).move(event.delta);
              ref.read(resizingStateNotifierProvider(widget.viewId).notifier).resize(event.delta);
            };
            instance.onEnd = (_) {
              ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).endMove();
              ref.read(resizingStateNotifierProvider(widget.viewId).notifier).endResize();
              interactiveMoveSubscription?.close();
              interactiveResizeSubscription?.close();
            };
            instance.onCancel = () {
              ref.read(windowMoveStateNotifierProvider(widget.viewId).notifier).cancelMove();
              ref.read(resizingStateNotifierProvider(widget.viewId).notifier).cancelResize();
              interactiveMoveSubscription?.close();
              interactiveResizeSubscription?.close();
            };
          },
        ),
      },
      child: widget.child,
    );
  }
}
