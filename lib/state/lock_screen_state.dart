import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'lock_screen_state.freezed.dart';

final lockScreenStateProvider =
    StateNotifierProvider<LockScreenStateNotifier, LockScreenState>((ref) => LockScreenStateNotifier());

class LockScreenStateNotifier extends StateNotifier<LockScreenState> {
  LockScreenStateNotifier()
      : super(const LockScreenState(
          dragging: false,
          dragVelocity: 0.0,
          offset: 0.0,
          slideDistance: 100.0,
        ));

  void startDrag() {
    state = state.copyWith(
      dragging: true,
    );
  }

  void drag(double offset) {
    state = state.copyWith(
      offset: (state.offset + offset).clamp(0, state.slideDistance),
    );
  }

  void endDrag(double velocity) {
    state = state.copyWith(
      dragging: false,
      dragVelocity: velocity,
    );
  }
}

@freezed
class LockScreenState with _$LockScreenState {
  const factory LockScreenState({
    required bool dragging,
    required double dragVelocity,
    required double offset,
    required double slideDistance,
  }) = _LockScreenState;
}
