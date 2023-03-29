import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/popup_stack.dart';
import 'package:zenith/ui/common/state/zenith_subsurface_state.dart';
import 'package:zenith/ui/common/state/zenith_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_popup_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/ui/common/subsurface.dart';
import 'package:zenith/util/state_notifier_list.dart';

final mappedWindowListProvider = StateNotifierProvider<StateNotifierList<int>, List<int>>((ref) {
  return StateNotifierList<int>();
});

final windowMappedStreamProvider = StreamProvider<int>((ref) {
  return PlatformApi.windowMappedController.stream;
});

final windowUnmappedStreamProvider = StreamProvider<int>((ref) {
  return PlatformApi.windowUnmappedController.stream;
});

class PlatformApi {
  // TODO: These statics are really ugly. I should have a singleton instance in Riverpod.

  static late final ProviderContainer ref;

  static final StreamController<dynamic> textInputEventsStreamController = StreamController();

  static final textInputEventsStream = textInputEventsStreamController.stream.asBroadcastStream();

  static const MethodChannel platform = MethodChannel('platform');

  static final windowMappedController = StreamController<int>.broadcast();

  static final windowUnmappedController = StreamController<int>.broadcast();

  static void init(ProviderContainer ref) {
    PlatformApi.ref = ref;
    PlatformApi.platform.setMethodCallHandler((call) async {
      switch (call.method) {
        case "commit_surface":
          _commitSurface(call.arguments);
          break;
        case "map_xdg_surface":
          _mapXdgSurface(call.arguments);
          break;
        case "unmap_xdg_surface":
          _unmapXdgSurface(call.arguments);
          break;
        case "map_subsurface":
          _mapSubsurface(call.arguments);
          break;
        case "unmap_subsurface":
          _unmapSubsurface(call.arguments);
          break;
        case "send_text_input_event":
          _sendTextInputEvent(call.arguments);
          break;
        case "interactive_move":
          _interactiveMove(call.arguments);
          break;
        case "interactive_resize":
          _interactiveResize(call.arguments);
          break;
        case "set_title":
          _setTitle(call.arguments);
          break;
        case "set_app_id":
          _setAppId(call.arguments);
          break;
        default:
          throw PlatformException(
            code: "unknown_method",
            message: "Unknown method ${call.method}",
          );
      }
    });
  }

  static Future<void> startupComplete() {
    return platform.invokeMethod("startup_complete");
  }

  static Future<void> pointerHoversView(int viewId, Offset position) {
    return platform.invokeMethod("pointer_hover", {
      "view_id": viewId,
      "x": position.dx,
      "y": position.dy,
    });
  }

  static Future<void> sendMouseButtonEventToView(int button, bool isPressed) {
    // One might find surprising that the view id is not sent to the platform. This is because the view id is only sent
    // when the pointer moves, and when a button event happens, the platform already knows which view it hovers.
    return platform.invokeMethod("mouse_button_event", {
      "button": button,
      "is_pressed": isPressed,
    });
  }

  static Future<void> pointerExitsView() {
    return platform.invokeMethod("pointer_exit");
  }

  static Future<void> activateWindow(int viewId) {
    return platform.invokeMethod('activate_window', viewId);
  }

  static Future<void> changeWindowVisibility(int viewId, bool visible) {
    return platform.invokeMethod('change_window_visibility', {
      "view_id": viewId,
      "visible": visible,
    });
  }

  static Future<void> unregisterViewTexture(int textureId) {
    return platform.invokeMethod('unregister_view_texture', textureId);
  }

  static Future<void> touchDown(int viewId, int touchId, Offset position) {
    return platform.invokeMethod('touch_down', {
      "view_id": viewId,
      "touch_id": touchId,
      "x": position.dx,
      "y": position.dy,
    });
  }

  static Future<void> touchMotion(int touchId, Offset position) {
    return platform.invokeMethod('touch_motion', {
      "touch_id": touchId,
      "x": position.dx,
      "y": position.dy,
    });
  }

  static Future<void> touchUp(int touchId) {
    return platform.invokeMethod('touch_up', {
      "touch_id": touchId,
    });
  }

  static Future<void> touchCancel(int touchId) {
    return platform.invokeMethod('touch_cancel', {
      "touch_id": touchId,
    });
  }

  static Future<void> insertText(int viewId, String text) {
    return platform.invokeMethod('insert_text', {
      "view_id": viewId,
      "text": text,
    });
  }

  static Future<void> emulateKeyCode(int viewId, int keyCode) {
    return platform.invokeMethod('emulate_keycode', {
      "view_id": viewId,
      "keycode": keyCode,
    });
  }

  static Future<void> startWindowsMaximized(bool value) {
    return platform.invokeMethod("start_windows_maximized", value);
  }

  static Future<void> maximizedWindowSize(int width, int height) {
    return platform.invokeMethod("maximized_window_size", {
      "width": width,
      "height": height,
    });
  }

  static Future<void> maximizeWindow(int viewId, bool value) {
    return platform.invokeMethod("maximize_window", {
      "view_id": viewId,
      "value": value,
    });
  }

  static Future<void> resizeWindow(int viewId, int width, int height) {
    return platform.invokeMethod("resize_window", {
      "view_id": viewId,
      "width": width,
      "height": height,
    });
  }

  static Stream<TextInputEventType> getTextInputEventsForViewId(int viewId) {
    return PlatformApi.textInputEventsStream.where((event) => event["view_id"] == viewId).map((event) {
      switch (event["type"]) {
        case "enable":
          return TextInputEnable();
        case "disable":
          return TextInputDisable();
        case "commit":
          return TextInputCommit();
        default:
          throw ArgumentError.value(event["type"], "Must be 'enable', 'disable', or 'commit'", "event['type']");
      }
    });
  }

  static Future<void> closeView(int viewId) {
    return platform.invokeMethod("close_window", {
      "view_id": viewId,
    });
  }

  static Future<AuthenticationResponse> unlockSession(String password) async {
    Map<String, dynamic>? response = await platform.invokeMapMethod("unlock_session", {
      "password": password,
    });
    if (response == null) {
      return AuthenticationResponse(false, "");
    }
    return AuthenticationResponse(response["success"] as bool, response["message"] as String);
  }

  /// The display will not generate frame events anymore if it's disabled, meaning that rendering is stopped.
  static Future<void> enableDisplay(bool enable) async {
    return platform.invokeMethod("enable_display", {
      "enable": enable,
    });
  }

  static void _commitSurface(dynamic event) {
    int viewId = event["view_id"];
    dynamic surface = event["surface"];
    int role = surface["role"];
    int textureId = surface["textureId"];
    int x = surface["x"];
    int y = surface["y"];
    int width = surface["width"];
    int height = surface["height"];
    int scale = surface["scale"];

    dynamic inputRegion = surface["input_region"];
    int left = inputRegion["x1"];
    int top = inputRegion["y1"];
    int right = inputRegion["x2"];
    int bottom = inputRegion["y2"];
    var inputRegionRect = Rect.fromLTRB(
      left.toDouble(),
      top.toDouble(),
      right.toDouble(),
      bottom.toDouble(),
    );

    List<dynamic> subsurfacesBelow = surface["subsurfaces_below"];
    List<dynamic> subsurfacesAbove = surface["subsurfaces_above"];

    List<int> subsurfaceIdsBelow = [];
    List<int> subsurfaceIdsAbove = [];

    for (dynamic subsurface in subsurfacesBelow) {
      int id = subsurface["id"];
      int x = subsurface["x"];
      int y = subsurface["y"];

      subsurfaceIdsBelow.add(id);

      var position = Offset(x.toDouble(), y.toDouble());
      ref.read(zenithSubsurfaceStateProvider(id).notifier).commit(position: position);
    }

    for (dynamic subsurface in subsurfacesAbove) {
      int id = subsurface["id"];
      int x = subsurface["x"];
      int y = subsurface["y"];

      subsurfaceIdsAbove.add(id);

      var position = Offset(x.toDouble(), y.toDouble());
      ref.read(zenithSubsurfaceStateProvider(id).notifier).commit(position: position);
    }

    ref.read(zenithSurfaceStateProvider(viewId).notifier).commit(
          role: SurfaceRole.values[role],
          textureId: textureId,
          surfacePosition: Offset(x.toDouble(), y.toDouble()),
          surfaceSize: Size(width.toDouble(), height.toDouble()),
          scale: scale.toDouble(),
          subsurfacesBelow: subsurfaceIdsBelow,
          subsurfacesAbove: subsurfaceIdsAbove,
          inputRegion: inputRegionRect,
        );

    bool hasXdgSurface = event["has_xdg_surface"];
    if (hasXdgSurface) {
      dynamic xdgSurface = event["xdg_surface"];
      int role = xdgSurface["role"];
      int x = xdgSurface["x"];
      int y = xdgSurface["y"];
      int width = xdgSurface["width"];
      int height = xdgSurface["height"];

      ref.read(zenithXdgSurfaceStateProvider(viewId).notifier).commit(
            role: XdgSurfaceRole.values[role],
            visibleBounds: Rect.fromLTWH(
              x.toDouble(),
              y.toDouble(),
              width.toDouble(),
              height.toDouble(),
            ),
          );

      bool hasXdgPopup = event["has_xdg_popup"];
      if (hasXdgPopup) {
        dynamic xdgPopup = event["xdg_popup"];
        int parentId = xdgPopup["parent_id"];
        int x = xdgPopup["x"];
        int y = xdgPopup["y"];
        int width = xdgPopup["width"];
        int height = xdgPopup["height"];

        ref.read(zenithXdgPopupStateProvider(viewId).notifier).commit(
              parentViewId: parentId,
              position: Offset(x.toDouble(), y.toDouble()),
            );
      }
    }

    bool hasToplevelDecoration = event["has_toplevel_decoration"];
    if (hasToplevelDecoration) {
      int toplevelDecorationInt = event["toplevel_decoration"];
      var decoration = ToplevelDecoration.fromInt(toplevelDecorationInt);
      ref.read(zenithXdgToplevelStateProvider(viewId).notifier).setDecoration(decoration);
    }

    bool hasToplevelTitle = event["has_toplevel_title"];
    if (hasToplevelTitle) {
      String title = event["toplevel_title"];
      ref.read(zenithXdgToplevelStateProvider(viewId).notifier).setTitle(title);
    }

    bool hasToplevelAppId = event["has_toplevel_app_id"];
    if (hasToplevelAppId) {
      String appId = event["toplevel_app_id"];
      ref.read(zenithXdgToplevelStateProvider(viewId).notifier).setAppId(appId);
    }
  }

  static void _mapXdgSurface(dynamic event) {
    int viewId = event["view_id"];

    XdgSurfaceRole role = ref.read(zenithXdgSurfaceStateProvider(viewId)).role;
    switch (role) {
      case XdgSurfaceRole.none:
        if (kDebugMode) {
          assert(false);
          print("unreachable");
          print(StackTrace.current);
        }
        break;
      case XdgSurfaceRole.toplevel:
        ref.read(mappedWindowListProvider.notifier).add(viewId);
        windowMappedController.add(viewId);
        break;
      case XdgSurfaceRole.popup:
        ref.read(popupStackChildren.notifier).add(viewId);
        break;
    }
  }

  static void _unmapXdgSurface(dynamic event) async {
    int viewId = event["view_id"];

    XdgSurfaceRole role = ref.read(zenithXdgSurfaceStateProvider(viewId)).role;
    switch (role) {
      case XdgSurfaceRole.none:
        if (kDebugMode) {
          assert(false);
          print("unreachable");
          print(StackTrace.current);
        }
        break; // Unreachable.
      case XdgSurfaceRole.toplevel:
        ref.read(mappedWindowListProvider.notifier).remove(viewId);
        windowUnmappedController.add(viewId);
        break;
      case XdgSurfaceRole.popup:
        await ref.read(zenithXdgPopupStateProvider(viewId).notifier).animateClosing();
        PlatformApi.unregisterViewTexture(ref.read(zenithSurfaceStateProvider(viewId)).textureId);
        ref.read(popupStackChildren.notifier).remove(viewId);
        break;
    }
  }

  static void _mapSubsurface(dynamic event) {
    int viewId = event["view_id"];

    ref.read(zenithSubsurfaceStateProvider(viewId).notifier).map(true);
  }

  static void _unmapSubsurface(dynamic event) {
    int viewId = event["view_id"];

    ref.read(zenithSubsurfaceStateProvider(viewId).notifier).map(false);
    ref.invalidate(subsurfaceWidget(viewId));
  }

  static void _sendTextInputEvent(dynamic event) {
    textInputEventsStreamController.sink.add(event);
  }

  static void _interactiveMove(dynamic event) {
    int viewId = event["view_id"];
    ref.read(zenithXdgToplevelStateProvider(viewId).notifier).requestInteractiveMove();
  }

  static void _interactiveResize(dynamic event) {
    int viewId = event["view_id"];
    int edge = event["edge"];
    ResizeEdge resizeEdge = ResizeEdge.fromInt(edge);
    ref.read(zenithXdgToplevelStateProvider(viewId).notifier).requestInteractiveResize(resizeEdge);
  }

  static void _setTitle(dynamic event) {
    int viewId = event["view_id"];
    String title = event["title"];
    ref.read(zenithXdgToplevelStateProvider(viewId).notifier).setTitle(title);
  }

  static void _setAppId(dynamic event) {
    int viewId = event["view_id"];
    String appId = event["app_id"];
    ref.read(zenithXdgToplevelStateProvider(viewId).notifier).setTitle(appId);
  }
}

abstract class TextInputEventType {}

class TextInputEnable extends TextInputEventType {}

class TextInputDisable extends TextInputEventType {}

class TextInputCommit extends TextInputEventType {}

class AuthenticationResponse {
  AuthenticationResponse(this.success, this.message);

  bool success;
  String message;
}
