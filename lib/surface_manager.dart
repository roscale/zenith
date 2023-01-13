import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/popup_state.dart';
import 'package:zenith/state/zenith_subsurface_state.dart';
import 'package:zenith/state/zenith_surface_state.dart';
import 'package:zenith/state/zenith_xdg_surface_state.dart';
import 'package:zenith/widgets/popup.dart';
import 'package:zenith/widgets/subsurface.dart';
import 'package:zenith/widgets/surface.dart';
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
      PlatformApi.surfaceCommitStream.listen(_surfaceCommit),
      PlatformApi.xdgSurfaceMapStream.listen(_xdgSurfaceMap),
      PlatformApi.xdgSurfaceUnmapStream.listen(_xdgSurfaceUnmap),
      PlatformApi.subsurfaceMapStream.listen(_subsurfaceMap),
      PlatformApi.subsurfaceUnmapStream.listen(_subsurfaceUnmap),
    ]);
  }

  void _surfaceCommit(dynamic event) {
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
      _ref.read(zenithSubsurfaceStateProvider(id).notifier).commit(position: position);
    }

    for (dynamic subsurface in subsurfacesAbove) {
      int id = subsurface["id"];
      int x = subsurface["x"];
      int y = subsurface["y"];

      subsurfaceIdsAbove.add(id);

      var position = Offset(x.toDouble(), y.toDouble());
      _ref.read(zenithSubsurfaceStateProvider(id).notifier).commit(position: position);
    }

    _ref.read(zenithSurfaceStateProvider(viewId).notifier).commit(
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

      _ref.read(zenithXdgSurfaceStateProvider(viewId).notifier).commit(
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

        _ref.read(popupStateProvider(viewId).notifier).commit(
              parentViewId: parentId,
              position: Offset(x.toDouble(), y.toDouble()),
            );
      }
    }

    _ref.read(surfaceWidget(viewId).notifier).state = Surface(
      key: _ref.read(zenithSurfaceStateProvider(viewId)).widgetKey,
      viewId: viewId,
    );
  }

  void _xdgSurfaceMap(dynamic event) {
    int viewId = event["view_id"];

    XdgSurfaceRole role = _ref.read(zenithXdgSurfaceStateProvider(viewId)).role;
    switch (role) {
      case XdgSurfaceRole.none:
        if (kDebugMode) {
          assert(false);
          print("unreachable");
          print(StackTrace.current);
        }
        break; // Unreachable.
      case XdgSurfaceRole.toplevel:
        _ref.read(windowWidget(viewId).notifier).state = Window(
          key: _ref.read(zenithXdgSurfaceStateProvider(viewId)).widgetKey,
          viewId: viewId,
        );
        _windowMappedController.add(viewId);
        break;
      case XdgSurfaceRole.popup:
        var popup = _ref.read(popupStateProvider(viewId));

        _ref.read(popupWidget(viewId).notifier).state = Popup(
          key: _ref.read(zenithXdgSurfaceStateProvider(viewId)).widgetKey,
          viewId: viewId,
        );

        _ref
            .read(zenithXdgSurfaceStateProvider(popup.parentViewId).notifier)
            .addPopup(viewId);

        break;
    }
  }

  void _xdgSurfaceUnmap(dynamic event) async {
    int viewId = event["view_id"];

    XdgSurfaceRole role = _ref.read(zenithXdgSurfaceStateProvider(viewId)).role;
    switch (role) {
      case XdgSurfaceRole.none:
        if (kDebugMode) {
          assert(false);
          print("unreachable");
          print(StackTrace.current);
        }
        break; // Unreachable.
      case XdgSurfaceRole.toplevel:
        _windowUnmappedController.add(viewId);
        break;
      case XdgSurfaceRole.popup:
        await _ref.read(popupStateProvider(viewId).notifier).animateClosing();

        final state = _ref.read(popupStateProvider(viewId));
        _ref.read(zenithXdgSurfaceStateProvider(state.parentViewId).notifier).removePopup(viewId);
        break;
    }
  }

  void _subsurfaceMap(dynamic event) {
    int viewId = event["view_id"];

    _ref.read(zenithSubsurfaceStateProvider(viewId).notifier).map(true);
    _ref.read(subsurfaceWidget(viewId).notifier).state = Subsurface(
      key: _ref.read(zenithSubsurfaceStateProvider(viewId)).widgetKey,
      viewId: viewId,
    );
  }

  void _subsurfaceUnmap(dynamic event) {
    int viewId = event["view_id"];

    _ref.read(zenithSubsurfaceStateProvider(viewId).notifier).map(false);
    _ref.invalidate(subsurfaceWidget(viewId));
  }

  void dispose() {
    for (var subscription in _streamSubscriptions) {
      subscription.cancel();
    }
    _windowMappedController.close();
    _windowUnmappedController.close();
  }
}
