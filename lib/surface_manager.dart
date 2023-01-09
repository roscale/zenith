import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/popup_state.dart';
import 'package:zenith/state/zenith_surface_state.dart';
import 'package:zenith/state/zenith_xdg_surface_state.dart';
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
      // PlatformApi.windowMappedStream.listen(_windowMapped),
      // PlatformApi.windowUnmappedStream.listen(_windowUnmapped),
      // PlatformApi.popupMappedStream.listen(_popupMapped),
      // PlatformApi.popupUnmappedStream.listen(_popupUnmapped),
      // PlatformApi.configureSurfaceStream.listen(_configureSurface),
      PlatformApi.surfaceCommitStream.listen(_surfaceCommit),
      PlatformApi.xdgSurfaceMapStream.listen(_xdgSurfaceMap),
      PlatformApi.xdgSurfaceUnmapStream.listen(_xdgSurfaceUnmap),
    ]);
  }

  // void _windowMapped(dynamic event) {
  //   // int viewId = event["view_id"];
  //   // int textureId = event["texture_id"];
  //   // int surfaceWidth = event["surface_width"];
  //   // int surfaceHeight = event["surface_height"];
  //   //
  //   // // Visible bounds relative to (0, 0) being the top left corner of the surface.
  //   // var visibleBoundsMap = Map<String, int>.from(event["visible_bounds"]);
  //   // var visibleBounds = Rect.fromLTWH(
  //   //   visibleBoundsMap["x"]!.toDouble(),
  //   //   visibleBoundsMap["y"]!.toDouble(),
  //   //   visibleBoundsMap["width"]!.toDouble(),
  //   //   visibleBoundsMap["height"]!.toDouble(),
  //   // );
  //   //
  //   // _ref.read(windowStateProvider(viewId).notifier).initialize(
  //   //       textureId: textureId,
  //   //       surfaceSize: Size(surfaceWidth.toDouble(), surfaceHeight.toDouble()),
  //   //       visibleBounds: visibleBounds,
  //   //     );
  //   //
  //   // _ref.read(windowWidget(viewId).notifier).state = Window(
  //   //   key: _ref.read(baseViewStateProvider(viewId)).widgetKey,
  //   //   viewId: viewId,
  //   // );
  //   //
  //   // _windowMappedController.add(viewId);
  // }

  // void _windowUnmapped(dynamic event) async {
  //   int viewId = event["view_id"];
  //   _windowUnmappedController.add(viewId);
  // }

  // TODO

  // void _popupMapped(dynamic event) {
  //   int viewId = event["view_id"];
  //   int textureId = event["texture_id"];
  //   int parentViewId = event["parent_view_id"];
  //   int x = event["x"];
  //   int y = event["y"];
  //   int width = event["surface_width"];
  //   int height = event["surface_height"];
  //   var visibleBoundsMap = event["visible_bounds"];
  //   var visibleBounds = Rect.fromLTWH(
  //     visibleBoundsMap["x"]!.toDouble(),
  //     visibleBoundsMap["y"]!.toDouble(),
  //     visibleBoundsMap["width"]!.toDouble(),
  //     visibleBoundsMap["height"]!.toDouble(),
  //   );
  //
  //   var parentVisibleBounds = _ref.read(baseViewStateProvider(parentViewId)).visibleBounds;
  //
  //   _ref.read(popupStateProvider(viewId).notifier).initialize(
  //         position: Offset(x.toDouble() + parentVisibleBounds.left, y.toDouble() + parentVisibleBounds.top),
  //         textureId: textureId,
  //         surfaceSize: Size(width.toDouble(), height.toDouble()),
  //         visibleBounds: visibleBounds,
  //       );
  //
  //   _ref.read(popupWidget(viewId).notifier).state = Popup(
  //     key: _ref.read(baseViewStateProvider(viewId)).widgetKey,
  //     viewId: viewId,
  //   );
  //
  //   _ref.read(baseViewStateProvider(parentViewId).notifier).addPopup(viewId);
  // }

  // TODO

  // void _popupUnmapped(dynamic event) async {
  //   int viewId = event["view_id"];
  //
  //   await _ref.read(popupStateProvider(viewId).notifier).animateClosing();
  //
  //   final state = _ref.read(popupStateProvider(viewId));
  //   _ref.read(baseViewStateProvider(state.parentViewId).notifier).removePopup(viewId);
  //
  //   _ref.invalidate(baseViewStateProvider(viewId));
  //   _ref.invalidate(popupStateProvider(viewId));
  //   _ref.invalidate(popupWidget(viewId));
  //
  //   PlatformApi.unregisterViewTexture(_ref.read(baseViewStateProvider(viewId)).textureId);
  // }

  // void _configureSurface(dynamic event) {
  //   int viewId = event["view_id"];
  //   XdgSurfaceRole role = XdgSurfaceRole.values[event["surface_role"]];
  //
  //   Size? newSurfaceSize;
  //   int? newTextureId;
  //   if (event["surface_size_changed"]) {
  //     newTextureId = event["texture_id"];
  //     int surfaceWidth = event["surface_width"];
  //     int surfaceHeight = event["surface_height"];
  //     newSurfaceSize = Size(surfaceWidth.toDouble(), surfaceHeight.toDouble());
  //   }
  //
  //   Rect? newVisibleBounds;
  //   if (event["visible_bounds_changed"]) {
  //     var visibleBoundsMap = event["visible_bounds"];
  //     newVisibleBounds = Rect.fromLTWH(
  //       visibleBoundsMap["x"]!.toDouble(),
  //       visibleBoundsMap["y"]!.toDouble(),
  //       visibleBoundsMap["width"]!.toDouble(),
  //       visibleBoundsMap["height"]!.toDouble(),
  //     );
  //   }
  //
  //   switch (role) {
  //     case XdgSurfaceRole.toplevel:
  //       final state = _ref.read(baseViewStateProvider(viewId));
  //       final notifier = _ref.read(baseViewStateProvider(viewId).notifier);
  //
  //       if (newTextureId != null) {
  //         final oldTextureId = state.textureId;
  //         SchedulerBinding.instance.addPostFrameCallback((_) {
  //           PlatformApi.unregisterViewTexture(oldTextureId);
  //         });
  //         notifier.textureId = newTextureId;
  //       }
  //       notifier.surfaceSize = newSurfaceSize ?? state.surfaceSize;
  //       notifier.visibleBounds = newVisibleBounds ?? state.visibleBounds;
  //       break;
  //
  //     case XdgSurfaceRole.popup:
  //       final state = _ref.read(baseViewStateProvider(viewId));
  //       final notifier = _ref.read(baseViewStateProvider(viewId).notifier);
  //
  //       if (newTextureId != null) {
  //         final oldTextureId = state.textureId;
  //         SchedulerBinding.instance.addPostFrameCallback((_) {
  //           PlatformApi.unregisterViewTexture(oldTextureId);
  //         });
  //         notifier.textureId = newTextureId;
  //       }
  //       notifier.surfaceSize = newSurfaceSize ?? state.surfaceSize;
  //       notifier.visibleBounds = newVisibleBounds ?? state.visibleBounds;
  //       if (event["popup_position_changed"]) {
  //         // Position relative to the parent.
  //         int x = event["x"];
  //         int y = event["y"];
  //         _ref.read(popupStateProvider(viewId).notifier).position =
  //             Offset(x.toDouble(), y.toDouble());
  //       }
  //       break;
  //
  //     case XdgSurfaceRole.none:
  //       assert(false, "xdg_surface has no role, this should never happen.");
  //       break;
  //   }
  // }

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

    _ref.read(zenithSurfaceStateProvider(viewId).notifier).commit(
          role: SurfaceRole.values[role],
          textureId: textureId,
          surfacePosition: Offset(x.toDouble(), y.toDouble()),
          surfaceSize: Size(width.toDouble(), height.toDouble()),
          scale: scale.toDouble(),
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
        _ref
            .read(zenithXdgSurfaceStateProvider(state.parentViewId).notifier)
            .removePopup(viewId);
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
