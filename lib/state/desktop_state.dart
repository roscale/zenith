import 'dart:async';
import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:zenith/enums.dart';
import 'package:zenith/state/popup_state.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/util/util.dart';
import 'package:zenith/widgets/desktop.dart';
import 'package:zenith/widgets/popup.dart';
import 'package:zenith/widgets/window.dart';

class DesktopState with ChangeNotifier {
  // Keeps track of all windows and popups alive. The widget is of type Window or Popup.
  Map<int, Widget> views = {};

  // Sends global pointer up events to windows. Listening for this event at the window level is not
  // reliable because sometimes it is not getting emitted when the pointer quickly leaves the window
  // which causes the window to be impossible to interact with.
  var pointerUpStream = StreamController<PointerUpEvent>.broadcast();

  static const EventChannel windowMappedEvent = EventChannel('window_mapped');
  static const EventChannel windowUnmappedEvent = EventChannel('window_unmapped');
  static const EventChannel popupMappedEvent = EventChannel('popup_mapped');
  static const EventChannel popupUnmappedEvent = EventChannel('popup_unmapped');
  static const EventChannel requestMoveEvent = EventChannel('request_move');
  static const EventChannel requestResizeEvent = EventChannel('request_resize');
  static const EventChannel configureSurfaceEvent = EventChannel('configure_surface');

  static const MethodChannel platform = MethodChannel('platform');

  Offset pointerPosition = window.physicalGeometry.center;

  DesktopState() {
    windowMappedEvent.receiveBroadcastStream().listen(windowMapped);
    windowUnmappedEvent.receiveBroadcastStream().listen(windowUnmapped);
    popupMappedEvent.receiveBroadcastStream().listen(popupMapped);
    popupUnmappedEvent.receiveBroadcastStream().listen(popupUnmapped);
    configureSurfaceEvent.receiveBroadcastStream().listen(configureSurface);
  }

  void windowMapped(dynamic event) {
    int viewId = event["view_id"];
    int surfaceWidth = event["surface_width"];
    int surfaceHeight = event["surface_height"];

    // Visible bounds relative to (0, 0) being the top left corner of the surface.
    var visibleBoundsMap = Map<String, int>.from(event["visible_bounds"]);
    var visibleBounds = Rect.fromLTWH(
      visibleBoundsMap["x"]!.toDouble(),
      visibleBoundsMap["y"]!.toDouble(),
      visibleBoundsMap["width"]!.toDouble(),
      visibleBoundsMap["height"]!.toDouble(),
    );

    var initialWindowPosition = Rect.fromCenter(
      center: pointerPosition,
      width: visibleBounds.width,
      height: visibleBounds.height,
    );
    initialWindowPosition = initialWindowPosition.clampTo(
      Rect.fromLTRB(
        window.physicalGeometry.left,
        window.physicalGeometry.top + 40,
        window.physicalGeometry.right,
        window.physicalGeometry.bottom,
      ),
    );

    var clientWindow = Window(WindowState(
      viewId: viewId,
      surfaceSize: Size(surfaceWidth.toDouble(), surfaceHeight.toDouble()),
      visibleBounds: visibleBounds,
    ));
    views[viewId] = clientWindow;
    taskSwitcherKey.currentState!.spawnTask(clientWindow);
    notifyListeners();
  }

  void windowUnmapped(dynamic event) async {
    int viewId = event["view_id"];
    var window = views[viewId] as Window;

    await taskSwitcherKey.currentState!.stopTask(window);
    views.remove(viewId);

    platform.invokeMethod('closing_animation_finished', window.state.viewId);
    notifyListeners();
  }

  void popupMapped(dynamic event) {
    int viewId = event["view_id"];
    int parentViewId = event["parent_view_id"];
    int x = event["x"];
    int y = event["y"];
    int width = event["surface_width"];
    int height = event["surface_height"];
    var visibleBoundsMap = event["visible_bounds"];
    var visibleBounds = Rect.fromLTWH(
      visibleBoundsMap["x"]!.toDouble(),
      visibleBoundsMap["y"]!.toDouble(),
      visibleBoundsMap["width"]!.toDouble(),
      visibleBoundsMap["height"]!.toDouble(),
    );

    var popup = Popup(PopupState(
      viewId: viewId,
      position: Offset(x.toDouble(), y.toDouble()),
      surfaceSize: Size(width.toDouble(), height.toDouble()),
      visibleBounds: visibleBounds,
    ));

    Widget parent = views[parentViewId]!;
    if (parent is Window) {
      parent.state.addPopup(popup);
    } else if (parent is Popup) {
      parent.state.addPopup(popup);
    }

    views[viewId] = popup;
    notifyListeners();
  }

  void popupUnmapped(dynamic event) async {
    int viewId = event["view_id"];

    var popup = views[viewId] as Popup;
    await popup.state.animateClosing();

    views.remove(viewId);

    Widget parent = views[popup.state.parentViewId]!;
    if (parent is Window) {
      parent.state.removePopup(popup);
    } else if (parent is Popup) {
      parent.state.removePopup(popup);
    }

    platform.invokeMethod('closing_animation_finished', popup.state.viewId);
    notifyListeners();
  }

  void configureSurface(dynamic event) {
    int viewId = event["view_id"];
    XdgSurfaceRole role = XdgSurfaceRole.values[event["surface_role"]];

    Size? newSurfaceSize;
    if (event["surface_size_changed"]) {
      int surfaceWidth = event["surface_width"];
      int surfaceHeight = event["surface_height"];
      newSurfaceSize = Size(surfaceWidth.toDouble(), surfaceHeight.toDouble());
    }

    Rect? newVisibleBounds;
    if (event["geometry_changed"]) {
      var visibleBoundsMap = event["visible_bounds"];
      newVisibleBounds = Rect.fromLTWH(
        visibleBoundsMap["x"]!.toDouble(),
        visibleBoundsMap["y"]!.toDouble(),
        visibleBoundsMap["width"]!.toDouble(),
        visibleBoundsMap["height"]!.toDouble(),
      );
    }

    switch (role) {
      case XdgSurfaceRole.toplevel:
        var window = views[viewId] as Window;
        window.state.surfaceSize = newSurfaceSize ?? window.state.surfaceSize;
        window.state.visibleBounds = newVisibleBounds ?? window.state.visibleBounds;
        break;

      case XdgSurfaceRole.popup:
        var popup = views[viewId] as Popup;
        popup.state.surfaceSize = newSurfaceSize ?? popup.state.surfaceSize;
        popup.state.visibleBounds = newVisibleBounds ?? popup.state.visibleBounds;
        if (event["popup_position_changed"]) {
          // Position relative to the parent.
          int x = event["x"];
          int y = event["y"];
          popup.state.position = Offset(x.toDouble(), y.toDouble());
        }
        break;

      case XdgSurfaceRole.none:
        assert(false, "xdg_surface has no role, this should never happen.");
        break;
    }
  }

  void activateWindow(int viewId) {
    var window = views[viewId] as Window;
    platform.invokeMethod('activate_window', window.state.viewId);
    notifyListeners();
  }
}
