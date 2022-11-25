import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'task_state.freezed.dart';

final taskStateProvider = StateNotifierProvider.family<TaskStateNotifier, TaskState, int>((ref, int viewId) {
  return TaskStateNotifier();
});

class TaskStateNotifier extends StateNotifier<TaskState> {
  TaskStateNotifier()
      : super(
          const TaskState(
            open: true,
            dismissState: TaskDismissState.open,
            startDismissAnimation: Object(),
            cancelDismissAnimation: Object(),
          ),
        );

  // TaskOpenState get openState => state.openState;

  set open(bool open) => state = state.copyWith(open: open);

  set dismissState(TaskDismissState dismissState) => state = state.copyWith(dismissState: dismissState);

  void startDismissAnimation() => state = state.copyWith(startDismissAnimation: Object());

  void cancelDismissAnimation() => state = state.copyWith(cancelDismissAnimation: Object());

  @override
  TaskState get state => super.state;
}

@freezed
class TaskState with _$TaskState {
  const factory TaskState({
    required bool open,
    required TaskDismissState dismissState,
    required Object startDismissAnimation,
    required Object cancelDismissAnimation,
  }) = _TaskState;
}

enum TaskDismissState {
  open,
  dismissing,
  dismissed,
}
