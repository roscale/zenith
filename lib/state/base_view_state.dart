import 'package:flutter/foundation.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/state/popup_state.dart';

part 'base_view_state.freezed.dart';

final baseViewState = StateNotifierProvider.family<BaseViewStateNotifier, BaseViewState, int>((ref, int viewId) {
  return BaseViewStateNotifier(ref, viewId);
});

/// Common state for both windows and popups.
@freezed
class BaseViewState with _$BaseViewState {
  const factory BaseViewState({
    required int viewId,
    required int textureId,
    required Size surfaceSize,
    required Rect visibleBounds,
    required List<int> popups,
    required Key widgetKey,
    required Key textureKey,
  }) = _BaseViewState;
}

class BaseViewStateNotifier extends StateNotifier<BaseViewState> {
  final Ref ref;

  BaseViewStateNotifier(this.ref, int viewId)
      : super(BaseViewState(
          viewId: viewId,
          textureId: -1,
          surfaceSize: Size.zero,
          visibleBounds: Rect.zero,
          popups: [],
          widgetKey: GlobalKey(),
          textureKey: GlobalKey(),
        ));

  void initialize({
    required int textureId,
    required Size surfaceSize,
    required Rect visibleBounds,
  }) {
    state = state.copyWith(
      textureId: textureId,
      surfaceSize: surfaceSize,
      visibleBounds: visibleBounds,
    );
  }

  set textureId(int value) {
    state = state.copyWith(
      textureId: value,
    );
  }

  set surfaceSize(Size value) {
    state = state.copyWith(surfaceSize: value);
  }

  set visibleBounds(Rect value) {
    state = state.copyWith(visibleBounds: value);
  }

  void addPopup(int viewId) {
    state = state.copyWith(popups: [...state.popups, viewId]);
    ref.read(popupState(viewId).notifier).parentViewId = state.viewId;
  }

  void removePopup(int viewId) {
    state = state.copyWith(popups: [
      for (int id in state.popups)
        if (id != viewId) id
    ]);
  }
}
