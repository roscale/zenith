import 'package:flutter/material.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:riverpod_annotation/riverpod_annotation.dart';
import 'package:zenith/ui/common/state/xdg_popup_state.dart';

part '../../../generated/ui/common/state/xdg_surface_state.freezed.dart';

part '../../../generated/ui/common/state/xdg_surface_state.g.dart';

@freezed
class XdgSurfaceState with _$XdgSurfaceState {
  const factory XdgSurfaceState({
    required XdgSurfaceRole role,
    required Rect visibleBounds,
    required GlobalKey widgetKey,
    required List<int> popups,
  }) = _XdgSurfaceState;
}

@Riverpod(keepAlive: true)
class XdgSurfaceStates extends _$XdgSurfaceStates {
  @override
  XdgSurfaceState build(int viewId) {
    return XdgSurfaceState(
      role: XdgSurfaceRole.none,
      visibleBounds: Rect.zero,
      widgetKey: GlobalKey(),
      popups: [],
    );
  }

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
    ref.read(xdgPopupStatesProvider(viewId).notifier).parentViewId = this.viewId;
  }

  void removePopup(int viewId) {
    state = state.copyWith(popups: [
      for (int id in state.popups)
        if (id != viewId) id
    ]);
  }
}

enum XdgSurfaceRole {
  none,
  toplevel,
  popup,
}
