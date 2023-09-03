import 'package:flutter/material.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:riverpod_annotation/riverpod_annotation.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/surface.dart';

part '../../../generated/ui/common/state/surface_state.freezed.dart';
part '../../../generated/ui/common/state/surface_state.g.dart';

@Riverpod(keepAlive: true)
Surface surfaceWidget(SurfaceWidgetRef ref, int viewId) {
  return Surface(
    key: ref.watch(surfaceStatesProvider(viewId).select((state) => state.widgetKey)),
    viewId: viewId,
  );
}

@freezed
class SurfaceState with _$SurfaceState {
  const factory SurfaceState({
    required SurfaceRole role,
    required int viewId,
    required TextureId textureId,
    required TextureId oldTextureId,
    required Offset surfacePosition,
    required Size surfaceSize,
    required double scale,
    required GlobalKey widgetKey,
    required GlobalKey textureKey,
    required List<int> subsurfacesBelow,
    required List<int> subsurfacesAbove,
    required Rect inputRegion,
  }) = _SurfaceState;
}

@Riverpod(keepAlive: true)
class SurfaceStates extends _$SurfaceStates {
  @override
  SurfaceState build(int viewId) {
    return SurfaceState(
      role: SurfaceRole.none,
      viewId: viewId,
      textureId: TextureId(-1),
      oldTextureId: TextureId(-1),
      surfacePosition: Offset.zero,
      surfaceSize: Size.zero,
      scale: 1,
      widgetKey: GlobalKey(),
      textureKey: GlobalKey(),
      subsurfacesBelow: [],
      subsurfacesAbove: [],
      inputRegion: Rect.zero,
    );
  }

  void commit({
    required SurfaceRole role,
    required TextureId textureId,
    required Offset surfacePosition,
    required Size surfaceSize,
    required double scale,
    required List<int> subsurfacesBelow,
    required List<int> subsurfacesAbove,
    required Rect inputRegion,
  }) {
    if (state.oldTextureId != textureId && state.oldTextureId != state.textureId) {
      final platform = ref.read(platformApiProvider.notifier);
      final id = state.oldTextureId.value;
      // Only unregister after the frame is rendered to avoid flickering.
      WidgetsBinding.instance.addPostFrameCallback((timeStamp) {
        platform.unregisterViewTexture(id);
      });
    }

    state = state.copyWith(
      role: role,
      textureId: textureId,
      oldTextureId: state.textureId,
      surfacePosition: surfacePosition,
      surfaceSize: surfaceSize,
      scale: scale,
      subsurfacesBelow: subsurfacesBelow,
      subsurfacesAbove: subsurfacesAbove,
      inputRegion: inputRegion,
    );
  }
}

enum SurfaceRole {
  none,
  xdgSurface,
  subsurface,
}
