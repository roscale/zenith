import 'package:flutter/cupertino.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/widgets/lock_screen.dart';

part 'lock_screen_state.freezed.dart';

final lockScreenStateProvider =
    StateNotifierProvider<LockScreenStateNotifier, LockScreenState>(
        (ref) => LockScreenStateNotifier());

class LockScreenStateNotifier extends StateNotifier<LockScreenState> {
  LockScreenStateNotifier()
      : super(
          LockScreenState(
            overlayEntry: OverlayEntry(builder: (_) => const LockScreen()),
            overlayEntryInserted: false,
            dragging: false,
            dragVelocity: 0.0,
            offset: 0.0,
            slideDistance: 300.0,
            lock: Object(),
            unlock: Object(),
            locked: false,
          ),
        );

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

  set offset(double offset) {
    state = state.copyWith(
      offset: offset,
    );
  }

  void endDrag(double velocity) {
    state = state.copyWith(
      dragging: false,
      dragVelocity: velocity,
    );
  }

  void lock() {
    state = state.copyWith(
      lock: Object(),
      locked: true,
      overlayEntryInserted: true,
      dragging: false,
      dragVelocity: 0.0,
      offset: 0.0,
    );
  }

  void unlock() {
    state = state.copyWith(
      unlock: Object(),
      locked: false,
    );
  }

  void removeOverlay() {
    state = state.copyWith(
      overlayEntryInserted: false,
    );
  }
}

@freezed
class LockScreenState with _$LockScreenState {
  const factory LockScreenState({
    required OverlayEntry overlayEntry,
    required bool overlayEntryInserted,
    required bool dragging,
    required double dragVelocity,
    required double offset,
    required double slideDistance,
    required Object lock,
    required Object unlock,
    required bool locked,
  }) = _LockScreenState;
}
