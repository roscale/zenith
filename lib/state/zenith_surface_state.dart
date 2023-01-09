import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'zenith_surface_state.freezed.dart';

final zenithSurfaceStateProvider = StateNotifierProvider.family<
    ZenithSurfaceStateNotifier, ZenithSurfaceState, int>((ref, int viewId) {
  return ZenithSurfaceStateNotifier(ref, viewId);
});

enum SurfaceRole {
  none,
  xdgSurface,
  subsurface,
}

@freezed
class ZenithSurfaceState with _$ZenithSurfaceState {
  const factory ZenithSurfaceState({
    required SurfaceRole role,
    required int viewId,
    required int textureId,
    required Offset surfacePosition,
    required Size surfaceSize,
    required double scale,
    required Key textureKey,
  }) = _ZenithSurfaceState;
}

class ZenithSurfaceStateNotifier extends StateNotifier<ZenithSurfaceState> {
  final Ref ref;

  ZenithSurfaceStateNotifier(this.ref, int viewId)
      : super(ZenithSurfaceState(
          role: SurfaceRole.none,
          viewId: viewId,
          textureId: -1,
          surfacePosition: Offset.zero,
          surfaceSize: Size.zero,
          scale: 1,
          textureKey: GlobalKey(),
        ));

  void commit({
    required SurfaceRole role,
    required int textureId,
    required Offset surfacePosition,
    required Size surfaceSize,
    required double scale,
  }) {
    state = state.copyWith(
      role: role,
      textureId: textureId,
      surfacePosition: surfacePosition,
      surfaceSize: surfaceSize,
      scale: scale,
    );
  }
}
