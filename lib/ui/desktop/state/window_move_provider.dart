import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'window_move_provider.freezed.dart';

final windowMoveProvider =
    StateNotifierProvider.family<WindowMoveStateNotifierProvider, WindowMoveState, int>((ref, int viewId) {
  return WindowMoveStateNotifierProvider(ref, viewId);
});

class WindowMoveStateNotifierProvider extends StateNotifier<WindowMoveState> {
  final Ref ref;
  final int viewId;

  WindowMoveStateNotifierProvider(this.ref, this.viewId)
      : super(
          const WindowMoveState(
            moving: false,
            startPosition: Offset.zero,
            movedPosition: Offset.zero,
            delta: Offset.zero,
          ),
        );

  void startPotentialMove() {
    state = state.copyWith(
      moving: false,
      delta: Offset.zero,
    );
  }

  void startMove(Offset position) {
    if (state.moving) {
      return;
    }

    state = state.copyWith(
      moving: true,
      startPosition: position,
    );
    if (state.delta != Offset.zero) {
      state = state.copyWith(
        movedPosition: state.startPosition + state.delta,
      );
    }
  }

  void move(Offset delta) {
    state = state.copyWith(
      delta: state.delta + delta,
    );
    if (state.moving) {
      state = state.copyWith(
        movedPosition: state.startPosition + state.delta,
      );
    }
  }

  void endMove() {
    if (!state.moving) {
      return;
    }
    state = state.copyWith(
      moving: false,
      delta: Offset.zero,
    );
  }

  void cancelMove() {
    if (!state.moving) {
      return;
    }
    state = state.copyWith(
      moving: false,
      delta: Offset.zero,
      movedPosition: state.startPosition,
    );
  }
}

@freezed
class WindowMoveState with _$WindowMoveState {
  const factory WindowMoveState({
    required bool moving,
    required Offset startPosition,
    required Offset movedPosition,
    required Offset delta,
  }) = _WindowMoveState;
}
