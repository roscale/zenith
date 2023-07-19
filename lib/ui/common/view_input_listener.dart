import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/zenith_surface_state.dart';
import 'package:zenith/util/mouse_button_tracker.dart';
import 'package:zenith/util/pointer_focus_manager.dart';
import 'package:zenith/util/raw_gesture_recognizer.dart';

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
    Rect inputRegion = ref.watch(zenithSurfaceStateProvider(widget.viewId).select((v) => v.inputRegion));

    return Stack(
      clipBehavior: Clip.none,
      children: [
        IgnorePointer(
          child: widget.child,
        ),
        Positioned.fromRect(
          rect: inputRegion,
          child: RawGestureDetector(
            gestures: <Type, GestureRecognizerFactory>{
              RawGestureRecognizer: GestureRecognizerFactoryWithHandlers<RawGestureRecognizer>(
                () => RawGestureRecognizer(debugOwner: this),
                (RawGestureRecognizer instance) {
                  instance.onPointerDown = (PointerDownEvent event) {
                    () async {
                      var position = event.localPosition + inputRegion.topLeft;

                      if (event.kind == PointerDeviceKind.mouse) {
                        await _pointerMoved(position);
                        await _sendMouseButtonsToPlatform(event.buttons);
                        pointerFocusManager.startPotentialDrag();
                      } else if (event.kind == PointerDeviceKind.touch) {
                        await PlatformApi.touchDown(widget.viewId, event.pointer, position);
                      }
                    }();
                    return null;
                  };
                  instance.onPointerMove = (PointerMoveEvent event, GestureDisposition? gestureDisposition) {
                    if (gestureDisposition == GestureDisposition.rejected) {
                      return;
                    }
                    () async {
                      var position = event.localPosition + inputRegion.topLeft;

                      if (event.kind == PointerDeviceKind.mouse) {
                        // If a button is being pressed while another one is already down, it's considered a move event, not a down event.
                        await _sendMouseButtonsToPlatform(event.buttons);
                        await _pointerMoved(position);
                      } else if (event.kind == PointerDeviceKind.touch) {
                        await PlatformApi.touchMotion(event.pointer, position);
                      }
                    }();
                    return null;
                  };
                  instance.onPointerUp = (PointerUpEvent event, GestureDisposition? gestureDisposition) {
                    if (gestureDisposition == GestureDisposition.rejected) {
                      return null;
                    }
                    () async {
                      if (event.kind == PointerDeviceKind.mouse) {
                        await _sendMouseButtonsToPlatform(event.buttons);
                        pointerFocusManager.stopPotentialDrag();
                      } else if (event.kind == PointerDeviceKind.touch) {
                        await PlatformApi.touchUp(event.pointer);
                      }
                    }();
                    return GestureDisposition.accepted;
                  };

                  Future<void> cancelPointer(PointerEvent event) async {
                    if (event.kind == PointerDeviceKind.mouse) {
                      await _sendMouseButtonsToPlatform(0);
                      pointerFocusManager.stopPotentialDrag();
                    } else if (event.kind == PointerDeviceKind.touch) {
                      await PlatformApi.touchCancel(event.pointer);
                    }
                  }

                  instance.onLose = (PointerEvent event) {
                    cancelPointer(event);
                  };
                },
              ),
            },
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

  Future<void> _sendMouseButtonsToPlatform(int buttons) async {
    MouseButtonEvent? e = mouseButtonTracker.trackButtonState(buttons);
    if (e != null) {
      await _mouseButtonEvent(e);
    }
  }

  Future<void> _mouseButtonEvent(MouseButtonEvent event) {
    return PlatformApi.sendMouseButtonEventToView(event.button, event.state == MouseButtonState.pressed);
  }

  Future<void> _pointerMoved(Offset position) {
    return PlatformApi.pointerHoversView(widget.viewId, position);
  }
}
