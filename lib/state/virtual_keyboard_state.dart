import 'package:flutter/rendering.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'virtual_keyboard_state.freezed.dart';

final virtualKeyboardStateProvider = StateNotifierProvider.autoDispose
    .family<VirtualKeyboardStateNotifier, VirtualKeyboardState, int>((ref, int viewId) {
  return VirtualKeyboardStateNotifier();
});

@freezed
class VirtualKeyboardState with _$VirtualKeyboardState {
  const factory VirtualKeyboardState({
    required bool activated,
    required Size keyboardSize,
  }) = _VirtualKeyboardState;
}

class VirtualKeyboardStateNotifier extends StateNotifier<VirtualKeyboardState> {
  VirtualKeyboardStateNotifier()
      : super(const VirtualKeyboardState(
          activated: false,
          keyboardSize: Size.zero,
        ));

  set activated(bool value) {
    state = state.copyWith(activated: value);
  }

  set keyboardSize(Size value) {
    state = state.copyWith(keyboardSize: value);
  }
}
