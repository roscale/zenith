import 'package:flutter/cupertino.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'task_state.freezed.dart';

final taskStateProvider = StateNotifierProvider.family<TaskStateNotifier, TaskState, int>((ref, int viewId) {
  return TaskStateNotifier();
});

final taskPositionProvider = StateProvider.family<double, int>((ref, int viewId) {
  return 0.0;
});

final taskVerticalPositionProvider = StateProvider.family<double, int>((ref, int viewId) {
  return 0.0;
});

final taskWidgetProvider = StateProvider.family<Widget, int>((ref, int viewId) {
  return const SizedBox();
});

class TaskStateNotifier extends StateNotifier<TaskState> {
  TaskStateNotifier()
      : super(
          const TaskState(
            dismissState: TaskDismissState.notDismissed,
            startDismissAnimation: Object(),
            cancelDismissAnimation: Object(),
          ),
        );

  set dismissState(TaskDismissState value) => state = state.copyWith(dismissState: value);

  void startDismissAnimation() => state = state.copyWith(startDismissAnimation: Object());

  void cancelDismissAnimation() => state = state.copyWith(cancelDismissAnimation: Object());

  @override
  TaskState get state => super.state;
}

@freezed
class TaskState with _$TaskState {
  const factory TaskState({
    required TaskDismissState dismissState,
    required Object startDismissAnimation,
    required Object cancelDismissAnimation,
  }) = _TaskState;
}

enum TaskDismissState {
  notDismissed,
  dismissing,
  dismissed,
}
