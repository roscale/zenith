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
            overlayKey: GlobalKey(),
            overlayEntry: OverlayEntry(builder: (_) => const LockScreen()),
            // The session starts locked. If you change this to false, also modify `initialEntries` of the Overlay widget.
            locked: true,
            lock: Object(),
            unlock: Object(),
            dragging: false,
            dragVelocity: 0.0,
            offset: 0.0,
            slideDistance: 300.0,
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
    if (!state.locked) {
      state.overlayKey.currentState?.insert(state.overlayEntry);
    }
    state = state.copyWith(
      lock: Object(),
      locked: true,
      dragging: false,
      dragVelocity: 0.0,
      offset: 0.0,
    );
  }

  /// Starts the unlock animation. The Widget must listen to this event.
  void unlock() {
    state = state.copyWith(
      unlock: Object(),
      locked: false,
    );
  }

  /// The Widget is responsible to call this method after the unlock animation is complete.
  void removeOverlay() {
    state.overlayEntry.remove();
  }
}

@freezed
class LockScreenState with _$LockScreenState {
  const factory LockScreenState({
    required GlobalKey<OverlayState> overlayKey,
    required OverlayEntry overlayEntry,
    required bool locked,
    required Object lock,
    required Object unlock,
    required bool dragging,
    required double dragVelocity,
    required double offset,
    required double slideDistance,
  }) = _LockScreenState;
}
