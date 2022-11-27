import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/state/base_view_state.dart';

part 'window_state.freezed.dart';

final windowStateProvider = StateNotifierProvider.family<WindowStateNotifier, WindowState, int>((ref, int viewId) {
  return WindowStateNotifier(ref, viewId);
});

@freezed
class WindowState with _$WindowState {
  const factory WindowState({
    required Key virtualKeyboardKey,
  }) = _WindowState;
}

class WindowStateNotifier extends StateNotifier<WindowState> {
  final Ref _ref;
  final int _viewId;

  WindowStateNotifier(this._ref, this._viewId)
      : super(WindowState(
          virtualKeyboardKey: GlobalKey(),
        ));

  void initialize({
    required int textureId,
    required Size surfaceSize,
    required Rect visibleBounds,
  }) {
    _ref.read(baseViewStateProvider(_viewId).notifier).initialize(
          textureId: textureId,
          surfaceSize: surfaceSize,
          visibleBounds: visibleBounds,
        );
  }
}
