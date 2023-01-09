import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/state/popup_state.dart';

part 'zenith_xdg_surface_state.freezed.dart';

final zenithXdgSurfaceStateProvider = StateNotifierProvider.family<
    ZenithXdgSurfaceStateNotifier,
    ZenithXdgSurfaceState,
    int>((ref, int viewId) {
  return ZenithXdgSurfaceStateNotifier(ref, viewId);
});

enum XdgSurfaceRole {
  none,
  toplevel,
  popup,
}

@freezed
class ZenithXdgSurfaceState with _$ZenithXdgSurfaceState {
  const factory ZenithXdgSurfaceState({
    required XdgSurfaceRole role,
    required Rect visibleBounds,
    required Key widgetKey,
    required List<int> popups,
  }) = _ZenithXdgSurfaceState;
}

class ZenithXdgSurfaceStateNotifier
    extends StateNotifier<ZenithXdgSurfaceState> {
  final Ref ref;
  final int _viewId;

  ZenithXdgSurfaceStateNotifier(this.ref, this._viewId)
      : super(ZenithXdgSurfaceState(
          role: XdgSurfaceRole.none,
          visibleBounds: Rect.zero,
          widgetKey: GlobalKey(),
          popups: [],
        ));

  void commit({
    required XdgSurfaceRole role,
    required Rect visibleBounds,
  }) {
    state = state.copyWith(
      role: role,
      visibleBounds: visibleBounds,
    );
  }

  void addPopup(int viewId) {
    state = state.copyWith(popups: [...state.popups, viewId]);
    ref.read(popupStateProvider(viewId).notifier).parentViewId = _viewId;
  }

  void removePopup(int viewId) {
    state = state.copyWith(popups: [
      for (int id in state.popups)
        if (id != viewId) id
    ]);
  }
}
