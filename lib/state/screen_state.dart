import 'package:flutter/scheduler.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/display_brightness_state.dart';
import 'package:zenith/state/lock_screen_state.dart';

part 'screen_state.freezed.dart';

final screenStateProvider = StateNotifierProvider<ScreenStateNotifier, ScreenState>((ref) {
  return ScreenStateNotifier(ref);
});

@freezed
class ScreenState with _$ScreenState {
  const factory ScreenState({
    required bool on,
    required bool pending, // Turn on/off operations have not yet finished.
  }) = _ScreenState;
}

class ScreenStateNotifier extends StateNotifier<ScreenState> {
  final Ref _ref;

  ScreenStateNotifier(this._ref)
      : super(
          const ScreenState(
            on: true,
            pending: false,
          ),
        );

  Future<void> turnOff() async {
    if (!state.on || state.pending) {
      return;
    }

    state = state.copyWith(pending: true);

    final displayBrightnessState = _ref.read(displayBrightnessStateProvider);
    final displayBrightnessStateNotifier = _ref.read(displayBrightnessStateProvider.notifier);

    if (displayBrightnessState.available) {
      displayBrightnessStateNotifier.saveBrightness();

      // There's a good reason we do these things in this order.
      // After turning off the screen, we wait one single frame to give the lock screen the opportunity to render
      // itself before we turn off rendering. The user will only see the lock screen when he turns the screen back on.
      try {
        // Multiple things can go wrong, the reason for this try-catch.
        // We might not have permission to write to the brightness file even if the file exists.
        await displayBrightnessStateNotifier.setBrightness(0);
        state = state.copyWith(on: false);
      } catch (_) {}

      SchedulerBinding.instance.addPostFrameCallback((_) {
        PlatformApi.enableDisplay(false);
        state = state.copyWith(pending: false);
      });
    }
  }

  Future<void> lockAndTurnOff() {
    _ref.read(lockScreenStateProvider.notifier).lock();
    return turnOff();
  }

  Future<void> turnOn() async {
    if (state.on || state.pending) {
      return;
    }

    state = state.copyWith(pending: true);

    final displayBrightnessState = _ref.read(displayBrightnessStateProvider);
    final displayBrightnessStateNotifier = _ref.read(displayBrightnessStateProvider.notifier);

    if (displayBrightnessState.available) {
      await PlatformApi.enableDisplay(true);
      SchedulerBinding.instance.addPostFrameCallback((_) async {
        try {
          await displayBrightnessStateNotifier.restoreBrightness();
          state = state.copyWith(on: true);
        } catch (_) {}

        state = state.copyWith(pending: false);
      });
    }
  }
}
