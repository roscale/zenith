import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/util/mouse_button_tracker.dart';
import 'package:zenith/util/pointer_focus_manager.dart';
import 'package:zenith/util/raw_gesture_recognizer.dart';

/// Handles all input events for a given window or popup, and redirects them to the platform which will them be
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
    // This is used instead of a Listener because there are other GestureDetectors on top of the view
    // that need priority when some system-wide gestures occur.
    // A Listener would always intercept events, but a [RawGestureRecognizer] will let other GestureDetectors handle the input.
    // onPointerCancel is called when this handover occurs.
    return RawGestureDetector(
      gestures: <Type, GestureRecognizerFactory>{
        RawGestureRecognizer: GestureRecognizerFactoryWithHandlers<RawGestureRecognizer>(
          () => RawGestureRecognizer(),
          (RawGestureRecognizer instance) {
            instance.onPointerDown = (PointerDownEvent event) async {
              if (event.kind == PointerDeviceKind.mouse) {
                await pointerMoved(event);
                await sendMouseEventsToPlatform(event);
                pointerFocusManager.startPotentialDrag();
              } else if (event.kind == PointerDeviceKind.touch) {
                await PlatformApi.touchDown(
                    widget.viewId, event.pointer, event.localPosition.dx, event.localPosition.dy);
              }
            };
            instance.onPointerMove = (PointerMoveEvent event) async {
              if (event.kind == PointerDeviceKind.mouse) {
                // If a button is being pressed while another one is already down, it's considered a move event, not a down event.
                await sendMouseEventsToPlatform(event);
                await pointerMoved(event);
              } else if (event.kind == PointerDeviceKind.touch) {
                await PlatformApi.touchMotion(event.pointer, event.localPosition.dx, event.localPosition.dy);
              }
            };
            instance.onPointerUp = (PointerUpEvent event) async {
              if (event.kind == PointerDeviceKind.mouse) {
                await sendMouseEventsToPlatform(event);
                pointerFocusManager.stopPotentialDrag();
              } else if (event.kind == PointerDeviceKind.touch) {
                await PlatformApi.touchUp(event.pointer);
              }
            };
            instance.onPointerCancel = (PointerCancelEvent event) async {
              if (event.kind == PointerDeviceKind.mouse) {
                await sendMouseEventsToPlatform(event);
                pointerFocusManager.stopPotentialDrag();
              } else if (event.kind == PointerDeviceKind.touch) {
                await PlatformApi.touchCancel(event.pointer);
              }
            };
          },
        ),
      },
      child: Listener(
        onPointerHover: (PointerHoverEvent event) {
          if (event.kind == PointerDeviceKind.mouse) {
            pointerMoved(event);
          }
        },
        child: MouseRegion(
          onEnter: (_) => pointerFocusManager.enterSurface(),
          onExit: (_) => pointerFocusManager.exitSurface(),
          child: widget.child,
        ),
      ),
    );
  }

  Future<void> sendMouseEventsToPlatform(PointerEvent event) async {
    if (event.kind == PointerDeviceKind.mouse) {
      MouseButtonEvent? e = mouseButtonTracker.trackButtonState(event.buttons);
      if (e != null) {
        await mouseButtonEvent(e);
      }
    }
  }

  Future<void> mouseButtonEvent(MouseButtonEvent event) {
    return PlatformApi.sendMouseButtonEventToView(event.button, event.state == MouseButtonState.pressed);
  }

  Future<void> pointerMoved(PointerEvent event) {
    return PlatformApi.pointerHoversView(widget.viewId, event.localPosition.dx, event.localPosition.dy);
  }
}
