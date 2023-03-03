import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'zenith_subsurface_state.freezed.dart';

final zenithSubsurfaceStateProvider = StateNotifierProvider.family<
    ZenithSubsurfaceStateNotifier,
    ZenithSubsurfaceState,
    int>((ref, int viewId) {
  return ZenithSubsurfaceStateNotifier(ref, viewId);
});

@freezed
class ZenithSubsurfaceState with _$ZenithSubsurfaceState {
  const factory ZenithSubsurfaceState({
    required Offset position, // relative to the parent
    required bool mapped,
    required Key widgetKey,
  }) = _ZenithSubsurfaceState;
}

class ZenithSubsurfaceStateNotifier
    extends StateNotifier<ZenithSubsurfaceState> {
  final Ref ref;

  ZenithSubsurfaceStateNotifier(this.ref, int viewId)
      : super(ZenithSubsurfaceState(
          position: Offset.zero,
          mapped: false,
          widgetKey: GlobalKey(),
        ));

  void commit({
    required Offset position,
  }) {
    state = state.copyWith(
      position: position,
    );
  }

  void map(bool value) {
    state = state.copyWith(
      mapped: value,
    );
  }
}
