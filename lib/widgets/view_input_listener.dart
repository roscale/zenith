import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/services.dart';
import 'package:zenith/util/mouse_button_tracker.dart';
import 'package:zenith/util/pointer_focus_manager.dart';

/// Handles all input events for a given window or popup, and redirects them to the platform which will them be
/// forwarded to the appropriate surface.
class ViewInputListener extends StatelessWidget {
  final int viewId;
  final Widget child;

  late final pointerFocusManager = getIt<PointerFocusManager>();
  late final mouseButtonTracker = getIt<MouseButtonTracker>();

  ViewInputListener({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Listener(
      onPointerDown: (PointerDownEvent event) async {
        await pointerMoved(event);
        await sendMouseEventsToPlatform(event);
        pointerFocusManager.startPotentialDrag();
      },
      onPointerUp: (PointerUpEvent event) async {
        await sendMouseEventsToPlatform(event);
        pointerFocusManager.stopPotentialDrag();
      },
      onPointerMove: (PointerMoveEvent event) async {
        // If a button is being pressed while another one is already down, it's considered a move event, not a down event.
        await sendMouseEventsToPlatform(event);
        await pointerMoved(event);
      },
      onPointerHover: (PointerHoverEvent event) => pointerMoved(event),
      child: MouseRegion(
        onEnter: (_) => pointerFocusManager.enterSurface(),
        onExit: (_) => pointerFocusManager.exitSurface(),
        child: child,
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
    return PlatformApi.pointerHoversView(viewId, event.localPosition.dx, event.localPosition.dy);
  }
}
