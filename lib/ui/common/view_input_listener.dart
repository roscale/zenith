import 'package:arena_listener/arena_listener.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/surface_state.dart';
import 'package:zenith/util/mouse_button_tracker.dart';
import 'package:zenith/util/pointer_focus_manager.dart';

/// Handles all input events for a given window or popup, and redirects them to the platform which will then be
/// forwarded to the appropriate surface.
class ViewInputListener extends ConsumerStatefulWidget {
  final int viewId;
  final Widget child;

  const ViewInputListener({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: key);

  @override
  ConsumerState<ViewInputListener> createState() => _ViewInputListenerState();
}

class _ViewInputListenerState extends ConsumerState<ViewInputListener> {
  late final pointerFocusManager = ref.read(pointerFocusManagerProvider);
  late final mouseButtonTracker = ref.read(mouseButtonTrackerProvider);

  @override
  Widget build(BuildContext context) {
    Rect inputRegion = ref.watch(surfaceStatesProvider(widget.viewId).select((v) => v.inputRegion));

    return Stack(
      clipBehavior: Clip.none,
      children: [
        IgnorePointer(
          child: widget.child,
        ),
        Positioned.fromRect(
          rect: inputRegion,
          child: ArenaListener(
            onPointerDown: (PointerDownEvent event) {
              _onPointerDown(event, inputRegion.topLeft);
              return null;
            },
            onPointerMove: (PointerMoveEvent event, GestureDisposition? disposition) {
              if (disposition == GestureDisposition.rejected) {
                return;
              }
              _onPointerMove(event, inputRegion.topLeft);
              return null;
            },
            onPointerUp: (PointerUpEvent event, GestureDisposition? disposition) {
              if (disposition == GestureDisposition.rejected) {
                return null;
              }
              _onPointerUp(event);
              return GestureDisposition.accepted;
            },
            onPointerCancel: (_, __) {
              return GestureDisposition.rejected;
            },
            onLose: _onLoseArena,
            child: Listener(
              onPointerHover: (PointerHoverEvent event) {
                if (event.kind == PointerDeviceKind.mouse) {
                  var position = event.localPosition + inputRegion.topLeft;
                  _pointerMoved(position);
                }
              },
              child: MouseRegion(
                onEnter: (_) => pointerFocusManager.enterSurface(),
                onExit: (_) => pointerFocusManager.exitSurface(),
              ),
            ),
          ),
        ),
      ],
    );
  }

  Future<void> _onPointerDown(PointerEvent event, Offset inputRegionTopLeft) async {
    var position = event.localPosition + inputRegionTopLeft;

    if (event.kind == PointerDeviceKind.mouse) {
      await _pointerMoved(position);
      await _sendMouseButtonsToPlatform(event.buttons);
      pointerFocusManager.startPotentialDrag();
    } else if (event.kind == PointerDeviceKind.touch) {
      await ref.read(platformApiProvider.notifier).touchDown(widget.viewId, event.pointer, position);
    }
  }

  Future<void> _onPointerMove(PointerEvent event, Offset inputRegionTopLeft) async {
    var position = event.localPosition + inputRegionTopLeft;

    if (event.kind == PointerDeviceKind.mouse) {
      // If a button is being pressed while another one is already down, it's considered a move event, not a down event.
      await _sendMouseButtonsToPlatform(event.buttons);
      await _pointerMoved(position);
    } else if (event.kind == PointerDeviceKind.touch) {
      await ref.read(platformApiProvider.notifier).touchMotion(event.pointer, position);
    }
  }

  Future<void> _onPointerUp(PointerUpEvent event) async {
    if (event.kind == PointerDeviceKind.mouse) {
      await _sendMouseButtonsToPlatform(event.buttons);
      pointerFocusManager.stopPotentialDrag();
    } else if (event.kind == PointerDeviceKind.touch) {
      await ref.read(platformApiProvider.notifier).touchUp(event.pointer);
    }
  }

  void _onLoseArena(PointerEvent lastPointerEvent) async {
    if (lastPointerEvent.kind == PointerDeviceKind.mouse) {
      await _sendMouseButtonsToPlatform(0);
      pointerFocusManager.stopPotentialDrag();
    } else if (lastPointerEvent.kind == PointerDeviceKind.touch) {
      await ref.read(platformApiProvider.notifier).touchCancel(lastPointerEvent.pointer);
    }
  }

  Future<void> _sendMouseButtonsToPlatform(int buttons) async {
    MouseButtonEvent? e = mouseButtonTracker.trackButtonState(buttons);
    if (e != null) {
      await _mouseButtonEvent(e);
    }
  }

  Future<void> _mouseButtonEvent(MouseButtonEvent event) {
    return ref.read(platformApiProvider.notifier).sendMouseButtonEventToView(event.button, event.state == MouseButtonState.pressed);
  }

  Future<void> _pointerMoved(Offset position) {
    return ref.read(platformApiProvider.notifier).pointerHoversView(widget.viewId, position);
  }
}
