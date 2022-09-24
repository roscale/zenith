import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/state/task_switcher_state.dart';
import 'package:zenith/system_ui/task_switcher/task_switcher.dart';

class InvisibleBottomBar extends ConsumerStatefulWidget {
  const InvisibleBottomBar({Key? key}) : super(key: key);

  @override
  ConsumerState<InvisibleBottomBar> createState() => _InvisibleBottomBarState();
}

class _InvisibleBottomBarState extends ConsumerState<InvisibleBottomBar> {
  late VelocityTracker velocityTracker;
  ScrollDragController? drag;
  late var tm = ref.read(taskSwitcherWidgetState);
  int draggingTask = 0;

  @override
  Widget build(BuildContext context) {
    return Consumer(
      builder: (_, WidgetRef ref, Widget? child) {
        final disableUserControl = ref.watch(taskSwitcherState.select((v) => v.disableUserControl));
        return AbsorbPointer(
          absorbing: disableUserControl,
          child: child!,
        );
      },
      child: GestureDetector(
        behavior: HitTestBehavior.translucent,
        onPanStart: _onPointerDown,
        onPanUpdate: _onPointerMove,
        onPanEnd: _onPointerUp,
        child: IgnorePointer(
          child: Container(
            height: 20,
            color: Colors.transparent,
          ),
        ),
      ),
    );
  }

  void _onPointerDown(DragStartDetails details) {
    tm.stopAnimations();
    velocityTracker = VelocityTracker.withKind(PointerDeviceKind.touch);
    velocityTracker.addPosition(details.sourceTimeStamp!, details.globalPosition);
    drag = tm.scrollPosition.drag(
      DragStartDetails(
        sourceTimeStamp: details.sourceTimeStamp!,
        globalPosition: details.globalPosition,
        localPosition: details.localPosition,
        kind: details.kind,
      ),
      _disposeDrag,
    ) as ScrollDragController;
    draggingTask = tm.positionToTaskIndex(tm.position);
  }

  void _onPointerMove(DragUpdateDetails details) {
    if (drag == null) {
      return;
    }
    final notifier = ref.read(taskSwitcherState.notifier);

    velocityTracker.addPosition(details.sourceTimeStamp!, details.globalPosition);
    if (ref.read(taskList).isNotEmpty) {
      double scale = ref.read(taskSwitcherState).scale;
      notifier.scale = (scale + details.delta.dy / ref.read(taskSwitcherState).constraints.maxHeight * 2).clamp(0.5, 1);
      scale = ref.read(taskSwitcherState).scale;

      drag?.update(DragUpdateDetails(
        sourceTimeStamp: details.sourceTimeStamp!,
        globalPosition: details.globalPosition,
        delta: Offset(details.delta.dx / scale, 0),
        primaryDelta: details.delta.dx / scale,
      ));
    }
  }

  void _onPointerUp(DragEndDetails details) {
    final tasks = ref.read(taskList);

    if (drag == null) {
      return;
    }
    drag?.cancel();

    // velocityTracker.addPosition(details.timeStamp, details.position);
    var vel = velocityTracker.getVelocity().pixelsPerSecond;

    var taskOffset = tm.taskIndexToPosition(tm.positionToTaskIndex(tm.position));

    if (vel.dx.abs() > 365 && vel.dx.abs() > vel.dy.abs()) {
      // Flick to the left or right.
      int targetTaskIndex;
      if (vel.dx < 0 && tm.position > taskOffset) {
        // Next task.
        targetTaskIndex = (tm.positionToTaskIndex(tm.position) + 1).clamp(0, tasks.length - 1);
      } else if (vel.dx > 0 && tm.position < taskOffset) {
        // Previous task.
        targetTaskIndex = (tm.positionToTaskIndex(tm.position) - 1).clamp(0, tasks.length - 1);
      } else {
        // Same task.
        targetTaskIndex = tm.positionToTaskIndex(tm.position);
      }
      tm.switchToTaskByIndex(targetTaskIndex);
    } else if (vel.dy < -200) {
      // Flick up.
      tm.showOverview();
    } else if (vel.dy > 200) {
      // Flick down.
      tm.switchToTaskByIndex(tm.positionToTaskIndex(tm.position));
    } else {
      // Lift finger while standing still.
      int taskInFront = tm.positionToTaskIndex(tm.position);
      if (taskInFront != draggingTask || ref.read(taskSwitcherState).scale > 0.9) {
        tm.switchToTaskByIndex(taskInFront);
      } else {
        tm.showOverview();
      }
    }
  }

  void _disposeDrag() {
    drag = null;
  }
}
