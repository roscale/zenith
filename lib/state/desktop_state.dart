import 'dart:async';

import 'package:flutter/material.dart';
import 'package:zenith/enums.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/popup_state.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/widgets/popup.dart';
import 'package:zenith/widgets/window.dart';

class DesktopState {
  final Set<StreamSubscription> _streamSubscriptions = {};

  // Keeps track of all windows and popups alive. The widget is of type Window or Popup.
  final Map<int, Widget> _views = {};

  final _windowMappedController = StreamController<Window>.broadcast();
  late final windowMappedStream = _windowMappedController.stream;

  final _windowUnmappedController = StreamController<Window>.broadcast();
  late final windowUnmappedStream = _windowUnmappedController.stream;

  DesktopState() {
    _streamSubscriptions.add(PlatformApi.windowMappedStream.listen(_windowMapped));
    _streamSubscriptions.add(PlatformApi.windowUnmappedStream.listen(_windowUnmapped));
    _streamSubscriptions.add(PlatformApi.popupMappedStream.listen(_popupMapped));
    _streamSubscriptions.add(PlatformApi.popupUnmappedStream.listen(_popupUnmapped));
    _streamSubscriptions.add(PlatformApi.configureSurfaceStream.listen(_configureSurface));
  }

  void dispose() {
    for (var subscription in _streamSubscriptions) {
      subscription.cancel();
    }
    _windowMappedController.close();
    _windowUnmappedController.close();
  }

  void _windowMapped(dynamic event) {
    int viewId = event["view_id"];
    int textureId = event["texture_id"];
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

    var clientWindow = Window(WindowState(
      viewId: viewId,
      textureId: textureId,
      surfaceSize: Size(surfaceWidth.toDouble(), surfaceHeight.toDouble()),
      visibleBounds: visibleBounds,
    ));
    _views[viewId] = clientWindow;

    _windowMappedController.add(clientWindow);
  }

  void _windowUnmapped(dynamic event) async {
    int viewId = event["view_id"];
    var window = _views[viewId] as Window;

    _windowUnmappedController.add(window);

    _views.remove(viewId);
  }

  void _popupMapped(dynamic event) {
    int viewId = event["view_id"];
    int textureId = event["texture_id"];
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

    var parentVisibleBounds = Rect.zero;
    Widget parent = _views[parentViewId]!;
    if (parent is Window) {
      parentVisibleBounds = parent.state.visibleBounds.value;
    } else if (parent is Popup) {
      parentVisibleBounds = parent.state.visibleBounds.value;
    }

    var popup = Popup(PopupState(
      viewId: viewId,
      textureId: textureId,
      position: Offset(x.toDouble() + parentVisibleBounds.left, y.toDouble() + parentVisibleBounds.top),
      surfaceSize: Size(width.toDouble(), height.toDouble()),
      visibleBounds: visibleBounds,
    ));

    if (parent is Window) {
      parent.state.addPopup(popup);
    } else if (parent is Popup) {
      parent.state.addPopup(popup);
    }

    _views[viewId] = popup;
  }

  void _popupUnmapped(dynamic event) async {
    int viewId = event["view_id"];

    var popup = _views[viewId] as Popup;
    await popup.state.animateClosing();

    _views.remove(viewId);

    Widget? parent = _views[popup.state.parentViewId];
    if (parent is Window) {
      parent.state.removePopup(popup);
    } else if (parent is Popup) {
      parent.state.removePopup(popup);
    }

    PlatformApi.unregisterViewTexture(popup.state.textureId.value);
  }

  void _configureSurface(dynamic event) {
    int viewId = event["view_id"];
    XdgSurfaceRole role = XdgSurfaceRole.values[event["surface_role"]];

    Size? newSurfaceSize;
    int? newTextureId;
    if (event["surface_size_changed"]) {
      newTextureId = event["texture_id"];
      int surfaceWidth = event["surface_width"];
      int surfaceHeight = event["surface_height"];
      newSurfaceSize = Size(surfaceWidth.toDouble(), surfaceHeight.toDouble());
    }

    Rect? newVisibleBounds;
    if (event["visible_bounds_changed"]) {
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
        var window = _views[viewId] as Window;
        if (newTextureId != null) {
          PlatformApi.unregisterViewTexture(window.state.textureId.value);
          window.state.textureId.value = newTextureId;
        }
        window.state.surfaceSize.value = newSurfaceSize ?? window.state.surfaceSize.value;
        window.state.visibleBounds.value = newVisibleBounds ?? window.state.visibleBounds.value;
        break;

      case XdgSurfaceRole.popup:
        var popup = _views[viewId] as Popup;
        if (newTextureId != null) {
          PlatformApi.unregisterViewTexture(popup.state.textureId.value);
          popup.state.textureId.value = newTextureId;
        }
        popup.state.surfaceSize.value = newSurfaceSize ?? popup.state.surfaceSize.value;
        popup.state.visibleBounds.value = newVisibleBounds ?? popup.state.visibleBounds.value;
        if (event["popup_position_changed"]) {
          // Position relative to the parent.
          int x = event["x"];
          int y = event["y"];
          popup.state.position.value = Offset(x.toDouble(), y.toDouble());
        }
        break;

      case XdgSurfaceRole.none:
        assert(false, "xdg_surface has no role, this should never happen.");
        break;
    }
  }
}
