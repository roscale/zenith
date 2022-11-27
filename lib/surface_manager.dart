import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/enums.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/base_view_state.dart';
import 'package:zenith/state/popup_state.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/widgets/popup.dart';
import 'package:zenith/widgets/window.dart';

final _surfaceManagerProvider = Provider((ref) {
  final surfaceManager = SurfaceManager(ref);
  ref.onDispose(surfaceManager.dispose);
  return surfaceManager;
});

final windowMappedStreamProvider = StreamProvider<int>((ref) {
  final desktopState = ref.watch(_surfaceManagerProvider);
  return desktopState._windowMappedController.stream;
});

final windowUnmappedStreamProvider = StreamProvider<int>((ref) {
  final desktopState = ref.watch(_surfaceManagerProvider);
  return desktopState._windowUnmappedController.stream;
});

class SurfaceManager {
  final ProviderRef _ref;

  final Set<StreamSubscription> _streamSubscriptions = {};

  final _windowMappedController = StreamController<int>.broadcast();

  final _windowUnmappedController = StreamController<int>.broadcast();

  SurfaceManager(this._ref) {
    _streamSubscriptions.addAll([
      PlatformApi.windowMappedStream.listen(_windowMapped),
      PlatformApi.windowUnmappedStream.listen(_windowUnmapped),
      PlatformApi.popupMappedStream.listen(_popupMapped),
      PlatformApi.popupUnmappedStream.listen(_popupUnmapped),
      PlatformApi.configureSurfaceStream.listen(_configureSurface),
    ]);
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

    _ref.read(windowStateProvider(viewId).notifier).initialize(
          textureId: textureId,
          surfaceSize: Size(surfaceWidth.toDouble(), surfaceHeight.toDouble()),
          visibleBounds: visibleBounds,
        );

    _ref.read(windowWidget(viewId).notifier).state = Window(
      key: _ref.read(baseViewStateProvider(viewId)).widgetKey,
      viewId: viewId,
    );

    _windowMappedController.add(viewId);
  }

  void _windowUnmapped(dynamic event) async {
    int viewId = event["view_id"];
    _windowUnmappedController.add(viewId);
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

    var parentVisibleBounds = _ref.read(baseViewStateProvider(parentViewId)).visibleBounds;

    _ref.read(popupStateProvider(viewId).notifier).initialize(
          position: Offset(x.toDouble() + parentVisibleBounds.left, y.toDouble() + parentVisibleBounds.top),
          textureId: textureId,
          surfaceSize: Size(width.toDouble(), height.toDouble()),
          visibleBounds: visibleBounds,
        );

    _ref.read(popupWidget(viewId).notifier).state = Popup(
      key: _ref.read(baseViewStateProvider(viewId)).widgetKey,
      viewId: viewId,
    );

    _ref.read(baseViewStateProvider(parentViewId).notifier).addPopup(viewId);
  }

  void _popupUnmapped(dynamic event) async {
    int viewId = event["view_id"];

    await _ref.read(popupStateProvider(viewId).notifier).animateClosing();

    final state = _ref.read(popupStateProvider(viewId));
    _ref.read(baseViewStateProvider(state.parentViewId).notifier).removePopup(viewId);

    _ref.invalidate(baseViewStateProvider(viewId));
    _ref.invalidate(popupStateProvider(viewId));
    _ref.invalidate(popupWidget(viewId));

    PlatformApi.unregisterViewTexture(_ref.read(baseViewStateProvider(viewId)).textureId);
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
        final state = _ref.read(baseViewStateProvider(viewId));
        final notifier = _ref.read(baseViewStateProvider(viewId).notifier);

        if (newTextureId != null) {
          final oldTextureId = state.textureId;
          SchedulerBinding.instance.addPostFrameCallback((_) {
            PlatformApi.unregisterViewTexture(oldTextureId);
          });
          notifier.textureId = newTextureId;
        }
        notifier.surfaceSize = newSurfaceSize ?? state.surfaceSize;
        notifier.visibleBounds = newVisibleBounds ?? state.visibleBounds;
        break;

      case XdgSurfaceRole.popup:
        final state = _ref.read(baseViewStateProvider(viewId));
        final notifier = _ref.read(baseViewStateProvider(viewId).notifier);

        if (newTextureId != null) {
          final oldTextureId = state.textureId;
          SchedulerBinding.instance.addPostFrameCallback((_) {
            PlatformApi.unregisterViewTexture(oldTextureId);
          });
          notifier.textureId = newTextureId;
        }
        notifier.surfaceSize = newSurfaceSize ?? state.surfaceSize;
        notifier.visibleBounds = newVisibleBounds ?? state.visibleBounds;
        if (event["popup_position_changed"]) {
          // Position relative to the parent.
          int x = event["x"];
          int y = event["y"];
          _ref.read(popupStateProvider(viewId).notifier).position = Offset(x.toDouble(), y.toDouble());
        }
        break;

      case XdgSurfaceRole.none:
        assert(false, "xdg_surface has no role, this should never happen.");
        break;
    }
  }

  void dispose() {
    for (var subscription in _streamSubscriptions) {
      subscription.cancel();
    }
    _windowMappedController.close();
    _windowUnmappedController.close();
  }
}
