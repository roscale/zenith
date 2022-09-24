import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'task_switcher_state.freezed.dart';

final taskSwitcherState = StateNotifierProvider<TaskSwitcherStateNotifier, TaskSwitcherState>((ref) {
  return TaskSwitcherStateNotifier();
});

@freezed
class TaskSwitcherState with _$TaskSwitcherState {
  factory TaskSwitcherState({
    required bool inOverview,
    required double scale,
    required bool disableUserControl, // Disables the ability to switch between tasks using gestures.
    required bool areAnimationsPlaying,
    required BoxConstraints constraints,
  }) = _TaskSwitcherState;
}

class TaskSwitcherStateNotifier extends StateNotifier<TaskSwitcherState> {
  TaskSwitcherStateNotifier()
      : super(TaskSwitcherState(
          inOverview: false,
          scale: 1.0,
          disableUserControl: false,
          areAnimationsPlaying: false,
          constraints: const BoxConstraints(),
        ));

  set inOverview(bool value) {
    state = state.copyWith(inOverview: value);
  }

  set scale(double value) {
    state = state.copyWith(scale: value);
  }

  set disableUserControl(bool value) {
    state = state.copyWith(disableUserControl: value);
  }

  set areAnimationsPlaying(bool value) {
    state = state.copyWith(areAnimationsPlaying: value);
  }

  set constraints(BoxConstraints value) {
    state = state.copyWith(constraints: value);
  }
}
