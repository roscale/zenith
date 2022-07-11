import 'package:flutter/services.dart';

class PlatformApi {
  static final Stream windowMappedStream = const EventChannel('window_mapped').receiveBroadcastStream();
  static final Stream windowUnmappedStream = const EventChannel('window_unmapped').receiveBroadcastStream();
  static final Stream popupMappedStream = const EventChannel('popup_mapped').receiveBroadcastStream();
  static final Stream popupUnmappedStream = const EventChannel('popup_unmapped').receiveBroadcastStream();
  static final Stream requestMoveStream = const EventChannel('request_move').receiveBroadcastStream();
  static final Stream requestResizeStream = const EventChannel('request_resize').receiveBroadcastStream();
  static final Stream configureSurfaceStream = const EventChannel('configure_surface').receiveBroadcastStream();

  static const MethodChannel _platform = MethodChannel('platform');

  static Future<void> pointerHoversView(int viewId, double x, double y) {
    return _platform.invokeMethod("pointer_hover", {
      "view_id": viewId,
      "x": x,
      "y": y,
    });
  }

  static Future<void> sendMouseButtonEventToView(int button, bool isPressed) {
    // One might find surprising that the view id is not sent to the platform. This is because the view id is only sent
    // when the pointer moves, and when a button event happens, the platform already knows which view it hovers.
    return _platform.invokeMethod("mouse_button_event", {
      "button": button,
      "is_pressed": isPressed,
    });
  }

  static Future<void> pointerExitsView() {
    return _platform.invokeMethod("pointer_exit");
  }

  static Future<void> activateWindow(int viewId) {
    return _platform.invokeMethod('activate_window', viewId);
  }

  static Future<void> changeWindowVisibility(int viewId, bool visible) {
    return _platform.invokeMethod('change_window_visibility', {
      "view_id": viewId,
      "visible": visible,
    });
  }

  static Future<void> unregisterViewTexture(int viewId) {
    return _platform.invokeMethod('unregister_view_texture', viewId);
  }

  static Future<void> touchDown(int viewId, int touchId, double x, double y) {
    return _platform.invokeMethod('touch_down', {
      "view_id": viewId,
      "touch_id": touchId,
      "x": x,
      "y": y,
    });
  }

  static Future<void> touchMotion(int touchId, double x, double y) {
    return _platform.invokeMethod('touch_motion', {
      "touch_id": touchId,
      "x": x,
      "y": y,
    });
  }

  static Future<void> touchUp(int touchId) {
    return _platform.invokeMethod('touch_up', {
      "touch_id": touchId,
    });
  }
}
