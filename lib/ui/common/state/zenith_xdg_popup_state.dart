import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/ui/common/popup.dart';

part 'zenith_xdg_popup_state.freezed.dart';

final zenithXdgPopupStateProvider =
    StateNotifierProvider.family<ZenithXdgPopupStateNotifier, ZenithXdgPopupState, int>((ref, int viewId) {
  return ZenithXdgPopupStateNotifier(ref, viewId);
});

@freezed
class ZenithXdgPopupState with _$ZenithXdgPopupState {
  const factory ZenithXdgPopupState({
    required int parentViewId,
    required Offset position,
    required GlobalKey<AnimationsState> animationsKey,
    required bool isClosing,
  }) = _ZenithXdgPopupState;
}

class ZenithXdgPopupStateNotifier extends StateNotifier<ZenithXdgPopupState> {
  final Ref _ref;
  final int _viewId;

  ZenithXdgPopupStateNotifier(this._ref, this._viewId)
      : super(ZenithXdgPopupState(
          parentViewId: -1,
          position: Offset.zero,
          animationsKey: GlobalKey<AnimationsState>(),
          isClosing: false,
        ));

  void commit({
    required int parentViewId,
    required Offset position,
  }) {
    state = state.copyWith(
      parentViewId: parentViewId,
      position: position,
    );
  }

  set parentViewId(int value) {
    state = state.copyWith(parentViewId: value);
  }

  set position(Offset value) {
    state = state.copyWith(position: value);
  }

  FutureOr animateClosing() {
    state = state.copyWith(isClosing: true);
    return state.animationsKey.currentState?.controller.reverse();
  }
}
