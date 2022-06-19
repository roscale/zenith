import 'dart:async';
import 'dart:ui';

import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/enums.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/desktop_state.dart';
import 'package:zenith/system_ui/task_switcher/invisible_bottom_bar.dart';
import 'package:zenith/util/change_notifier_list.dart';
import 'package:zenith/widgets/window.dart';

class TaskSwitcher extends StatefulWidget {
  final double spacing;

  const TaskSwitcher({
    Key? key,
    required this.spacing,
  }) : super(key: key);

  @override
  State<TaskSwitcher> createState() => TaskSwitcherState();
}

class TaskSwitcherState extends State<TaskSwitcher> with TickerProviderStateMixin implements ScrollContext {
  final overview = ValueNotifier(false);
  final tasks = ChangeNotifierList<Window>();

  late final ScrollPosition scrollPosition;
  final scale = ValueNotifier(1.0);

  /// Disables the ability to switch between tasks using gestures.
  final disableUserControl = ValueNotifier(false);

  /// True while some animation is playing.
  final animationsOngoing = ValueNotifier(false);

  /// Absolute scroll offset from the left.
  double get pixels => scrollPosition.pixels;

  final constraints = ValueNotifier(const BoxConstraints(maxWidth: 0, maxHeight: 0));

  AnimationController? scaleAnimationController;

  final Set<StreamSubscription> _streamSubscriptions = {};

  @override
  void initState() {
    super.initState();

    scrollPosition = ScrollPositionWithSingleContext(
      physics: const BouncingScrollPhysics(),
      context: this,
      debugLabel: "TaskSwitcher.scrollPosition",
    );

    // Dummy dimensions that will properly be set by LayoutBuilder.
    scrollPosition.applyViewportDimension(1);
    scrollPosition.applyContentDimensions(0, 0);

    void updateContentDimensions() {
      scrollPosition.applyContentDimensions(0, tasks.length <= 1 ? 0 : (tasks.length - 1) * _taskToTaskOffset);
    }

    constraints.addListener(() {
      scrollPosition.applyViewportDimension(constraints.value.maxWidth);
      updateContentDimensions();
    });
    tasks.addListener(updateContentDimensions);

    // Avoid executing _spawnTask and _stopTask concurrently because it causes visual glitches.
    // Make sure the async tasks are executed one after the other.
    Future<void> _chain = Future.value(null);
    final desktopState = context.read<DesktopState>();
    _streamSubscriptions.add(desktopState.windowMappedStream.listen((task) {
      _chain = _chain.then((_) => _spawnTask(task));
    }));
    _streamSubscriptions.add(desktopState.windowUnmappedStream.listen((task) {
      _chain = _chain.then((_) => _stopTask(task));
    }));
  }

  @override
  void dispose() {
    for (var subscription in _streamSubscriptions) {
      subscription.cancel();
    }
    scaleAnimationController?.dispose();
    scrollPosition.dispose();
    overview.dispose();
    tasks.dispose();
    scale.dispose();
    disableUserControl.dispose();
    animationsOngoing.dispose();
    constraints.dispose();
    super.dispose();
  }

  double get _taskToTaskOffset => constraints.value.maxWidth + widget.spacing;

  int taskIndex(double translation) {
    // 0 is the center of the first task.
    var offset = translation + _taskToTaskOffset / 2;
    var index = offset ~/ _taskToTaskOffset;
    return index.clamp(0, tasks.length - 1);
  }

  double position(int taskIndex) => taskIndex * _taskToTaskOffset;

  @override
  Widget build(BuildContext context) {
    return Provider.value(
      value: this,
      child: LayoutBuilder(
        builder: (BuildContext context, BoxConstraints constraints) {
          this.constraints.value = constraints;

          return Stack(
            children: [
              Positioned.fill(
                child: _buildGestureDetector(
                  child: _buildTaskListTransforms(
                    child: _buildTaskList(),
                  ),
                ),
              ),
              const Positioned(
                bottom: 0,
                left: 0,
                right: 0,
                child: InvisibleBottomBar(),
              )
            ],
          );
        },
      ),
    );
  }

  Widget _buildTaskListTransforms({required Widget child}) {
    return AnimatedBuilder(
      animation: scale,
      builder: (_, Widget? child) {
        return Transform.scale(
          scale: scale.value,
          child: child,
        );
      },
      child: AnimatedBuilder(
        animation: scrollPosition,
        builder: (_, Widget? child) {
          return Transform.translate(
            offset: Offset(-pixels, 0),
            child: child,
          );
        },
        child: child,
      ),
    );
  }

  Widget _buildTaskList() {
    return DeferredPointerHandler(
      child: AnimatedBuilder(
        animation: tasks,
        builder: (_, __) {
          return Stack(
            clipBehavior: Clip.none,
            children: _buildTaskWidgets().toList(),
          );
        },
      ),
    );
  }

  Widget _buildGestureDetector({required Widget child}) {
    return ValueListenableBuilder(
      valueListenable: overview,
      builder: (_, bool overview, Widget? child) {
        return RawGestureDetector(
          behavior: overview ? HitTestBehavior.opaque : HitTestBehavior.deferToChild,
          gestures: <Type, GestureRecognizerFactory>{
            HorizontalDragGestureRecognizer: GestureRecognizerFactoryWithHandlers<HorizontalDragGestureRecognizer>(
              () => HorizontalDragGestureRecognizer(),
              (HorizontalDragGestureRecognizer instance) {
                instance
                  ..onDown = _handleDragDown
                  ..onStart = _handleDragStart
                  ..onUpdate = _handleDragUpdate
                  ..onEnd = _handleDragEnd
                  ..onCancel = _handleDragCancel;
              },
            ),
          },
          child: child,
        );
      },
      child: child,
    );
  }

  Iterable<Widget> _buildTaskWidgets() sync* {
    double position = 0;

    for (Window task in tasks) {
      var taskWidget = ValueListenableBuilder(
        valueListenable: overview,
        builder: (_, bool overview, Widget? child) {
          if (overview) {
            return GestureDetector(
              behavior: HitTestBehavior.opaque,
              onTap: () => _switchToTask(task),
              child: IgnorePointer(
                child: child,
              ),
            );
          } else {
            return Listener(
              child: child!,
              onPointerDown: (_) async {
                // Move the task to the end after the animations have finished.
                if (tasks.last == task) {
                  return;
                }
                disableUserControl.value = true;
                await animationsOngoing.waitUntil((bool value) => value == false);
                moveCurrentTaskToEnd();
                jumpToLastTask();
                disableUserControl.value = false;
              },
            );
          }
        },
        child: ConstrainedBox(
          constraints: constraints.value,
          child: Center(
            child: FittedBox(
              child: task,
            ),
          ),
        ),
      );

      yield Positioned(
        left: position,
        child: DeferPointer(child: taskWidget),
      );

      position += _taskToTaskOffset;
    }
  }

  Future<void> _spawnTask(Window task) {
    tasks.add(task);
    return switchToTaskByIndex(tasks.length - 1, zoomOut: true);
  }

  void moveCurrentTaskToEnd() {
    if (tasks.isNotEmpty) {
      int currentTaskIndex = taskIndex(pixels);
      // Make the current task in last one.
      var task = tasks.removeAt(currentTaskIndex);
      tasks.add(task);
    }
  }

  void jumpToLastTask() {
    scrollPosition.jumpTo(tasks.isNotEmpty ? position(tasks.length - 1) : 0);
  }

  Future<void> _stopTask(window) async {
    int closingTask = tasks.indexOf(window);
    int? focusingTask = taskToFocusAfterClosing(closingTask);
    // Might be null if there's no task left, in which case there's nothing to animate.
    if (focusingTask != null) {
      var currentTaskIndex = taskIndex(pixels);

      // Don't let the user interact with the task switcher while the animation is ongoing.
      disableUserControl.value = true;
      if (currentTaskIndex == closingTask) {
        await switchToTaskByIndex(focusingTask, zoomOut: true);
      } else {
        await switchToTaskByIndex(currentTaskIndex);
      }
      disableUserControl.value = false;
    }
    _removeTask(window);
    PlatformApi.unregisterViewTexture(window.state.viewId);
  }

  int? taskToFocusAfterClosing(int closingTaskIndex) {
    if (tasks.length <= 1) {
      return null;
    } else if (closingTaskIndex == 0) {
      return 1;
    } else {
      return closingTaskIndex - 1;
    }
  }

  void _removeTask(Window task) {
    var currentTaskIndex = taskIndex(pixels);
    var index = tasks.indexOf(task);
    tasks.remove(task);

    // If the task to remove is before the current one in the list, it will shift all next ones
    // to the left. Update the translation.
    if (index < currentTaskIndex) {
      scrollPosition.jumpTo(position(currentTaskIndex - 1));
    }
  }

  void stopAnimations() {
    scaleAnimationController?.stop();
    scaleAnimationController?.dispose();
    scaleAnimationController = null;
  }

  Future<void> switchToTaskByIndex(int index, {bool zoomOut = false}) async {
    overview.value = false;
    PlatformApi.activateWindow(tasks[index].state.viewId);

    stopAnimations();
    scaleAnimationController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    );

    final positionAnimationFuture = scrollPosition.animateTo(
      position(index),
      duration: const Duration(milliseconds: 400),
      curve: Curves.easeOutCubic,
    );

    Animatable<double> scaleAnimatable;
    if (zoomOut) {
      scaleAnimatable = TweenSequence([
        TweenSequenceItem(
          weight: 30,
          tween: Tween(begin: scale.value, end: 0.8),
        ),
        TweenSequenceItem(
          weight: 70,
          tween: Tween(begin: 0.8, end: 1.0).chain(CurveTween(curve: Curves.easeOut)),
        ),
      ]);
    } else {
      scaleAnimatable = Tween(
        begin: scale.value,
        end: 1.0,
      ).chain(CurveTween(curve: Curves.easeOutCubic));
    }

    final scaleAnimation = scaleAnimationController!.drive(scaleAnimatable);
    scaleAnimationController!.addListener(() => scale.value = scaleAnimation.value);

    final scaleAnimationFuture = scaleAnimationController!.forward().orCancel.catchError((_) => null);

    animationsOngoing.value = true;
    await Future.wait([positionAnimationFuture, scaleAnimationFuture]);
    animationsOngoing.value = false;
  }

  Future<void> _switchToTask(Window task) => switchToTaskByIndex(tasks.indexOf(task));

  Future<void> showOverview() async {
    overview.value = true;

    stopAnimations();
    scaleAnimationController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    );

    final positionAnimationFuture = scrollPosition.animateTo(
      position(taskIndex(pixels)),
      duration: const Duration(milliseconds: 400),
      curve: Curves.easeOutCubic,
    );

    final scaleAnimation = scaleAnimationController!.drive(
      Tween(
        begin: scale.value,
        end: 0.6,
      ).chain(CurveTween(curve: Curves.easeOutCubic)),
    );
    scaleAnimation.addListener(() => scale.value = scaleAnimation.value);

    final scaleAnimationFuture = scaleAnimationController!.forward().orCancel.catchError((_) => null);

    animationsOngoing.value = true;
    await Future.wait([positionAnimationFuture, scaleAnimationFuture]);
    animationsOngoing.value = false;
  }

  //
  // RawGestureDetector callbacks
  //

  ScrollDragController? _drag;
  ScrollHoldController? _hold;

  void _handleDragDown(DragDownDetails details) {
    if (!overview.value) {
      return;
    }
    assert(_drag == null);
    assert(_hold == null);
    _hold = scrollPosition.hold(_disposeHold);
  }

  void _handleDragStart(DragStartDetails details) {
    if (!overview.value) {
      return;
    }
    // It's possible for _hold to become null between _handleDragDown and
    // _handleDragStart, for example if some user code calls jumpTo or otherwise
    // triggers a new activity to begin.
    assert(_drag == null);
    _drag = scrollPosition.drag(details, _disposeDrag) as ScrollDragController;
    assert(_drag != null);
    assert(_hold == null);
  }

  void _handleDragUpdate(DragUpdateDetails details) {
    if (!overview.value) {
      return;
    }
    // _drag might be null if the drag activity ended and called _disposeDrag.
    assert(_hold == null || _drag == null);
    // _drag?.update(details);
    _drag?.update(DragUpdateDetails(
      sourceTimeStamp: details.sourceTimeStamp,
      globalPosition: details.globalPosition,
      localPosition: details.localPosition,
      delta: Offset(details.delta.dx / scale.value, details.delta.dy / scale.value),
      primaryDelta: details.primaryDelta != null ? details.primaryDelta! / scale.value : null,
    ));
  }

  void _handleDragEnd(DragEndDetails details) {
    // _drag might be null if the drag activity ended and called _disposeDrag.
    assert(_hold == null || _drag == null);
    _drag?.end(DragEndDetails(
      velocity: Velocity(pixelsPerSecond: details.velocity.pixelsPerSecond / scale.value),
      primaryVelocity: details.primaryVelocity != null ? details.primaryVelocity! / scale.value : null,
    ));
    assert(_drag == null);
  }

  void _handleDragCancel() {
    // _hold might be null if the drag started.
    // _drag might be null if the drag activity ended and called _disposeDrag.
    assert(_hold == null || _drag == null);
    _hold?.cancel();
    _drag?.cancel();
    assert(_hold == null);
    assert(_drag == null);
  }

  void _disposeHold() {
    _hold = null;
  }

  void _disposeDrag() {
    _drag = null;
  }

  //
  // ScrollContext overrides
  //

  @override
  AxisDirection get axisDirection => AxisDirection.right;

  @override
  BuildContext? get notificationContext => context;

  @override
  void saveOffset(double offset) {}

  @override
  void setCanDrag(bool value) {}

  @override
  void setIgnorePointer(bool value) {}

  @override
  void setSemanticsActions(Set<SemanticsAction> actions) {}

  @override
  BuildContext get storageContext => context;

  @override
  TickerProvider get vsync => this;
}
