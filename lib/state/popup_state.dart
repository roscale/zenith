import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/state/base_view_state.dart';
import 'package:zenith/widgets/popup.dart';

part 'popup_state.freezed.dart';

final popupStateProvider = StateNotifierProvider.family<PopupStateNotifier, PopupState, int>((ref, int viewId) {
  return PopupStateNotifier(ref, viewId);
});

@freezed
class PopupState with _$PopupState {
  const factory PopupState({
    required int parentViewId,
    required Offset position,
    required GlobalKey<AnimationsState> animationsKey,
    required bool isClosing,
  }) = _PopupState;
}

class PopupStateNotifier extends StateNotifier<PopupState> {
  final Ref _ref;
  final int _viewId;

  PopupStateNotifier(this._ref, this._viewId)
      : super(PopupState(
          parentViewId: -1,
          position: Offset.zero,
          animationsKey: GlobalKey<AnimationsState>(),
          isClosing: false,
        ));

  void initialize({
    required Offset position,
    required int textureId,
    required Size surfaceSize,
    required Rect visibleBounds,
  }) {
    state = state.copyWith(position: position);
    _ref.read(baseViewStateProvider(_viewId).notifier).initialize(
          textureId: textureId,
          surfaceSize: surfaceSize,
          visibleBounds: visibleBounds,
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
