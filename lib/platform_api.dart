import 'package:flutter/services.dart';

class PlatformApi {
  static final Stream windowMappedStream = const EventChannel('window_mapped').receiveBroadcastStream();
  static final Stream windowUnmappedStream = const EventChannel('window_unmapped').receiveBroadcastStream();
  static final Stream popupMappedStream = const EventChannel('popup_mapped').receiveBroadcastStream();
  static final Stream popupUnmappedStream = const EventChannel('popup_unmapped').receiveBroadcastStream();
  static final Stream requestMoveStream = const EventChannel('request_move').receiveBroadcastStream();
  static final Stream requestResizeStream = const EventChannel('request_resize').receiveBroadcastStream();
  static final Stream configureSurfaceStream = const EventChannel('configure_surface').receiveBroadcastStream();
  static final Stream textInputEventsStream = const EventChannel('text_input_events').receiveBroadcastStream();
  static final Stream surfaceCommitStream = const EventChannel('zenith_surface_commit').receiveBroadcastStream();
  static final Stream xdgSurfaceMapStream = const EventChannel('xdg_surface_map').receiveBroadcastStream();
  static final Stream xdgSurfaceUnmapStream = const EventChannel('xdg_surface_unmap').receiveBroadcastStream();
  static final Stream subsurfaceMapStream = const EventChannel('zenith_subsurface_map').receiveBroadcastStream();
  static final Stream subsurfaceUnmapStream = const EventChannel('zenith_subsurface_unmap').receiveBroadcastStream();

  static const MethodChannel _platform = MethodChannel('platform');

  static Future<void> startupComplete() {
    return _platform.invokeMethod("startup_complete");
  }

  static Future<void> pointerHoversView(int viewId, Offset position) {
    return _platform.invokeMethod("pointer_hover", {
      "view_id": viewId,
      "x": position.dx,
      "y": position.dy,
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

  static Future<void> unregisterViewTexture(int textureId) {
    return _platform.invokeMethod('unregister_view_texture', textureId);
  }

  static Future<void> touchDown(int viewId, int touchId, Offset position) {
    return _platform.invokeMethod('touch_down', {
      "view_id": viewId,
      "touch_id": touchId,
      "x": position.dx,
      "y": position.dy,
    });
  }

  static Future<void> touchMotion(int touchId, Offset position) {
    return _platform.invokeMethod('touch_motion', {
      "touch_id": touchId,
      "x": position.dx,
      "y": position.dy,
    });
  }

  static Future<void> touchUp(int touchId) {
    return _platform.invokeMethod('touch_up', {
      "touch_id": touchId,
    });
  }

  static Future<void> touchCancel(int touchId) {
    return _platform.invokeMethod('touch_cancel', {
      "touch_id": touchId,
    });
  }

  static Future<void> insertText(int viewId, String text) {
    return _platform.invokeMethod('insert_text', {
      "view_id": viewId,
      "text": text,
    });
  }

  static Future<void> emulateKeyCode(int viewId, int keyCode) {
    return _platform.invokeMethod('emulate_keycode', {
      "view_id": viewId,
      "keycode": keyCode,
    });
  }

  static Future<void> initial_window_size(int width, int height) {
    return _platform.invokeMethod("initial_window_size", {
      "width": width,
      "height": height,
    });
  }

  static Future<void> resizeWindow(int viewId, int width, int height) {
    return _platform.invokeMethod("resize_window", {
      "view_id": viewId,
      "width": width,
      "height": height,
    });
  }

  static Stream<TextInputEvent> getTextInputEventsForViewId(int viewId) {
    return PlatformApi.textInputEventsStream
        .where((event) => event["view_id"] == viewId)
        .map((event) {
      switch (event["type"]) {
        case "enable":
          return TextInputEnable();
        case "disable":
          return TextInputDisable();
        case "commit":
          return TextInputCommit();
        default:
          throw ArgumentError.value(event["type"],
              "Must be 'enable', 'disable', or 'commit'", "event['type']");
      }
    });
  }

  static Future<void> closeView(int viewId) {
    return _platform.invokeMethod("close_window", {
      "view_id": viewId,
    });
  }

  static Future<AuthenticationResponse> unlockSession(String password) async {
    Map<String, dynamic>? response = await _platform.invokeMapMethod("unlock_session", {
      "password": password,
    });
    if (response == null) {
      return AuthenticationResponse(false, "");
    }
    return AuthenticationResponse(response["success"] as bool, response["message"] as String);
  }

  /// The display will not generate frame events anymore if it's disabled, meaning that rendering is stopped.
  static Future<void> enableDisplay(bool enable) async {
    return _platform.invokeMethod("enable_display", {
      "enable": enable,
    });
  }
}

abstract class TextInputEvent {}

class TextInputEnable extends TextInputEvent {}

class TextInputDisable extends TextInputEvent {}

class TextInputCommit extends TextInputEvent {}

class AuthenticationResponse {
  AuthenticationResponse(this.success, this.message);

  bool success;
  String message;
}
