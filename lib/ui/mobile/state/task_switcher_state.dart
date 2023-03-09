import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'task_switcher_state.freezed.dart';

final taskSwitcherStateProvider = StateNotifierProvider<TaskSwitcherStateNotifier, TaskSwitcherState>((ref) {
  return TaskSwitcherStateNotifier(ref);
});

/// To ensure widgets access the latest value even during build, this cannot be a Riverpod provider.
/// Riverpod cannot set provider values during build.
/// We still use Riverpod to announce changes to this variable.
BoxConstraints taskSwitcherConstraints = BoxConstraints.tight(Size.zero);

@freezed
class TaskSwitcherState with _$TaskSwitcherState {
  factory TaskSwitcherState({
    required bool inOverview,
    required double scale,
    required bool disableUserControl, // Disables the ability to switch between tasks using gestures.
    required bool areAnimationsPlaying,
    required Object constraintsChanged,
  }) = _TaskSwitcherState;
}

class TaskSwitcherStateNotifier extends StateNotifier<TaskSwitcherState> {
  final Ref ref;

  TaskSwitcherStateNotifier(this.ref)
      : super(
          TaskSwitcherState(
            inOverview: false,
            scale: 1.0,
            disableUserControl: false,
            areAnimationsPlaying: false,
            constraintsChanged: Object(),
          ),
        );

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

  void constraintsHaveChanged() {
    state = state.copyWith(constraintsChanged: Object());
  }
}
