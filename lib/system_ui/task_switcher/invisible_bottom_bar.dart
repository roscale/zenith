import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/system_ui/task_switcher/task_switcher.dart';

class InvisibleBottomBar extends StatefulWidget {
  const InvisibleBottomBar({Key? key}) : super(key: key);

  @override
  State<InvisibleBottomBar> createState() => _InvisibleBottomBarState();
}

class _InvisibleBottomBarState extends State<InvisibleBottomBar> {
  late VelocityTracker velocityTracker;
  ScrollDragController? drag;
  late var tm = context.read<TaskSwitcherState>();
  int draggingTask = 0;

  @override
  Widget build(BuildContext context) {
    return ValueListenableBuilder(
      valueListenable: tm.disableUserControl,
      builder: (_, bool ignoreUserControl, Widget? child) {
        return AbsorbPointer(
          absorbing: ignoreUserControl,
          child: child!,
        );
      },
      child: Listener(
        onPointerDown: _onPointerDown,
        onPointerMove: _onPointerMove,
        onPointerUp: _onPointerUp,
        child: Container(
          height: 20,
          color: Colors.transparent,
        ),
      ),
    );
  }

  void _onPointerDown(PointerDownEvent details) {
    tm.stopAnimations();
    velocityTracker = VelocityTracker.withKind(PointerDeviceKind.touch);
    velocityTracker.addPosition(details.timeStamp, details.position);
    drag = tm.scrollPosition.drag(
      DragStartDetails(
        sourceTimeStamp: details.timeStamp,
        globalPosition: details.position,
        localPosition: details.localPosition,
        kind: details.kind,
      ),
      _disposeDrag,
    ) as ScrollDragController;
    draggingTask = tm.taskIndex(tm.pixels);
  }

  void _onPointerMove(PointerMoveEvent details) {
    if (drag == null) {
      return;
    }
    velocityTracker.addPosition(details.timeStamp, details.position);
    if (tm.tasks.isNotEmpty) {
      tm.scale.value += details.delta.dy / tm.constraints.value.maxHeight * 2;
      tm.scale.value = tm.scale.value.clamp(0.5, 1);
      drag?.update(DragUpdateDetails(
        sourceTimeStamp: details.timeStamp,
        globalPosition: details.position,
        delta: Offset(details.delta.dx / tm.scale.value, 0),
        primaryDelta: details.delta.dx / tm.scale.value,
      ));
    }
  }

  void _onPointerUp(PointerUpEvent details) {
    if (drag == null) {
      return;
    }
    drag?.cancel();

    velocityTracker.addPosition(details.timeStamp, details.position);
    var vel = velocityTracker.getVelocity().pixelsPerSecond;

    var taskOffset = tm.position(tm.taskIndex(tm.pixels));

    if (vel.dx.abs() > 365 && vel.dx.abs() > vel.dy.abs()) {
      // Flick to the left or right.
      int targetTaskIndex;
      if (vel.dx < 0 && tm.pixels > taskOffset) {
        // Next task.
        targetTaskIndex = (tm.taskIndex(tm.pixels) + 1).clamp(0, tm.tasks.length - 1);
      } else if (vel.dx > 0 && tm.pixels < taskOffset) {
        // Previous task.
        targetTaskIndex = (tm.taskIndex(tm.pixels) - 1).clamp(0, tm.tasks.length - 1);
      } else {
        // Same task.
        targetTaskIndex = tm.taskIndex(tm.pixels);
      }
      tm.switchToTaskByIndex(targetTaskIndex);
    } else if (vel.dy < -200) {
      // Flick up.
      tm.showOverview();
    } else if (vel.dy > 200) {
      // Flick down.
      tm.switchToTaskByIndex(tm.taskIndex(tm.pixels));
    } else {
      // Lift finger while standing still.
      int taskInFront = tm.taskIndex(tm.pixels);
      if (taskInFront != draggingTask || tm.scale.value > 0.9) {
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
