import 'dart:async';
import 'dart:ui';

import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/base_view_state.dart';
import 'package:zenith/state/task_state.dart';
import 'package:zenith/state/task_switcher_state.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/surface_manager.dart';
import 'package:zenith/system_ui/app_drawer/handle.dart';
import 'package:zenith/system_ui/task_switcher/invisible_bottom_bar.dart';
import 'package:zenith/system_ui/task_switcher/task.dart';
import 'package:zenith/system_ui/task_switcher/task_switcher_scroller.dart';
import 'package:zenith/system_ui/task_switcher/task_switcher_viewport.dart';
import 'package:zenith/util/state_notifier_list.dart';
import 'package:zenith/widgets/window.dart';

final taskListProvider = StateNotifierProvider<StateNotifierList<int>, List<int>>((ref) {
  return StateNotifierList<int>();
});

final closingTaskListProvider = StateNotifierProvider<StateNotifierList<int>, List<int>>((ref) {
  return StateNotifierList<int>();
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

final taskSwitcherWidgetStateProvider = StateProvider((ref) => _TaskSwitcherState());

class TaskSwitcher extends ConsumerStatefulWidget {
  final double spacing;

  const TaskSwitcher({
    Key? key,
    required this.spacing,
  }) : super(key: key);

  @override
  ConsumerState<TaskSwitcher> createState() => _TaskSwitcherState();
}

class _TaskSwitcherState extends ConsumerState<TaskSwitcher> with TickerProviderStateMixin implements ScrollContext {
  late final ScrollPosition scrollPosition;

  double get position => scrollPosition.pixels;

  AnimationController? _scaleAnimationController;

  final Set<StreamSubscription> _streamSubscriptions = {};

  AnimationController? _taskPositionAnimationController;

  @override
  void initState() {
    super.initState();

    Future.microtask(() => ref.read(taskSwitcherWidgetStateProvider.notifier).state = this);

    scrollPosition = ScrollPositionWithSingleContext(
      physics: const BouncingScrollPhysics(),
      context: this,
      debugLabel: "TaskSwitcher.scrollPosition",
    );

    ref.listenManual(taskSwitcherState.select((v) => v.constraints), (_, BoxConstraints constraints) {
      scrollPosition.applyViewportDimension(constraints.maxWidth);
      _updateContentDimensions();
    });

    // ref.listenManual(taskList, (_, List<int> next) {
    //   for (int i = 0; i < next.length; i++) {
    //     ref.read(taskPositionProvider(next[i]).notifier).state = taskIndexToPosition(i);
    //   }
    //   updateContentDimensions();
    // });

    // Avoid executing _spawnTask and _stopTask concurrently because it causes visual glitches.
    // Make sure the async tasks are executed one after the other.
    Future<void> chain = Future.value(null);

    ref.listenManual(windowMappedStreamProvider, (_, AsyncValue<int> next) {
      next.whenData((int viewId) {
        chain = chain.then((_) => _spawnTask(viewId));
      });
    });

    ref.listenManual(windowUnmappedStreamProvider, (_, AsyncValue<int> next) {
      next.whenData((int viewId) {
        chain = chain.then((_) => _stopTask(viewId));
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (BuildContext context, BoxConstraints constraints) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          // addPostFrameCallback needed because it triggers setState which cannot be called during build.
          ref.read(taskSwitcherState.notifier).constraints = constraints;
          _repositionTasks();
          scrollPosition.jumpTo(taskIndexToPosition(positionToTaskIndex(position)));
        });

        PlatformApi.initial_window_size(constraints.maxWidth.toInt(), constraints.maxHeight.toInt());

        return Stack(
          children: [
            Positioned.fill(
              child: TaskSwitcherScroller(
                scrollPosition: scrollPosition,
                child: Stack(
                  children: [
                    const Positioned(
                      bottom: 0,
                      left: 0,
                      right: 0,
                      child: AppDrawerHandle(),
                    ),
                    TaskSwitcherViewport(
                      scrollPosition: scrollPosition,
                      child: DeferredPointerHandler(
                        child: Consumer(
                          builder: (_, WidgetRef ref, __) {
                            final tasks = ref.watch(taskListProvider);
                            final closingTasks = ref.watch(closingTaskListProvider);
                            return Stack(
                              clipBehavior: Clip.none,
                              children: [
                                for (int viewId in closingTasks) ref.watch(taskWidgetProvider(viewId)),
                                for (int viewId in tasks) ref.watch(taskWidgetProvider(viewId)),
                              ],
                            );
                          },
                        ),
                      ),
                    ),
                  ],
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
    );
  }

  void _updateContentDimensions({double? minScrollExtent}) {
    int length = ref.read(taskListProvider).length;
    minScrollExtent = minScrollExtent ?? (scrollPosition.hasContentDimensions ? scrollPosition.minScrollExtent : 0);
    scrollPosition.applyContentDimensions(
      minScrollExtent,
      minScrollExtent + (length <= 1 ? 0 : (length - 1) * _taskToTaskOffset),
    );
  }

  void _repositionTasks() {
    final List<int> tasks = ref.read(taskListProvider);
    for (int i = 0; i < tasks.length; i++) {
      ref.read(taskPositionProvider(tasks[i]).notifier).state = taskIndexToPosition(i);
    }
  }

  double get _taskToTaskOffset => ref.read(taskSwitcherState).constraints.maxWidth + widget.spacing;

  double taskIndexToPosition(int taskIndex) => scrollPosition.minScrollExtent + taskIndex * _taskToTaskOffset;

  int positionToTaskIndex(double position) {
    final taskListLength = ref.read(taskListProvider).length;
    if (taskListLength == 0) {
      return 0;
    }
    // 0 is the center of the first task.
    var offset = (position - scrollPosition.minScrollExtent) + _taskToTaskOffset / 2;
    var index = offset ~/ _taskToTaskOffset;
    return index.clamp(0, taskListLength - 1);
  }

  Future<void> _spawnTask(int viewId) {
    final taskIndex = ref.read(taskListProvider).length;

    ref.read(taskListProvider.notifier).add(viewId);
    ref.read(taskWidgetProvider(viewId).notifier).state = Task(
      key: ValueKey(viewId),
      viewId: viewId,
      onTap: () => _switchToTask(viewId),
      onClosed: () {},
    );
    ref.read(taskPositionProvider(viewId).notifier).state = taskIndexToPosition(taskIndex);

    _updateContentDimensions();

    filter(TaskState state) => state.open;
    listener(_, bool next) async {
      // bool open = next.item1;
      // TaskDismissState dismissState = next.item2;
      bool open = next;

      bool inOverview = ref.read(taskSwitcherState).inOverview;
      final tasks = ref.read(taskListProvider);

      if (!open && inOverview) {
        var closingTaskIndex = tasks.indexOf(viewId);

        _taskPositionAnimationController?.dispose();
        _taskPositionAnimationController =
            AnimationController(vsync: this, duration: const Duration(milliseconds: 300));
        final controller = _taskPositionAnimationController!;

        int taskIndexUnder = positionToTaskIndex(position);

        bool animateRight = false;
        if (taskIndexUnder == closingTaskIndex) {
          if (closingTaskIndex == tasks.length - 1) {
            // Closing the last task in the list.
            animateRight = false;
          } else {
            animateRight = true;
          }
        } else if (closingTaskIndex > taskIndexUnder) {
          animateRight = true;
        } else if (closingTaskIndex < taskIndexUnder) {
          animateRight = false;
        }

        if (animateRight) {
          for (int i = closingTaskIndex + 1; i < tasks.length; i++) {
            Animation<double> animation = Tween(
              begin: ref.read(taskPositionProvider(tasks[i])),
              end: taskIndexToPosition(i - 1),
            ).animate(CurvedAnimation(
              parent: controller,
              curve: Curves.easeOutCubic,
            ));

            controller.addListener(() {
              ref.read(taskPositionProvider(tasks[i]).notifier).state = animation.value;
            });
          }

          _removeTask(viewId);
          _updateContentDimensions();
        } else {
          for (int i = 0; i < closingTaskIndex; i++) {
            Animation<double> animation = Tween(
              begin: ref.read(taskPositionProvider(tasks[i])),
              end: taskIndexToPosition(i + 1),
            ).animate(CurvedAnimation(
              parent: controller,
              curve: Curves.easeOutCubic,
            ));

            controller.addListener(() {
              ref.read(taskPositionProvider(tasks[i]).notifier).state = animation.value;
            });
          }

          _removeTask(viewId);
          _updateContentDimensions(minScrollExtent: scrollPosition.minScrollExtent + _taskToTaskOffset);
        }

        // await Future.delayed(Duration(microseconds: 1));
        await controller.forward(from: 0).orCancel.catchError((_) => null);
        _destroyTask(viewId);

        // _repositionTasks();
      }
    }

    ref.listenManual(taskStateProvider(viewId).select(filter), listener);

    return switchToTaskByIndex(taskIndex, zoomOut: true);
  }

  void moveCurrentTaskToEnd() {
    if (ref.read(taskListProvider).isNotEmpty) {
      int currentTaskIndex = positionToTaskIndex(position);
      // Make the current task in last one.
      final notifier = ref.read(taskListProvider.notifier);
      var task = notifier.removeAt(currentTaskIndex);
      notifier.add(task);
    }
  }

  void jumpToLastTask() {
    final tasks = ref.read(taskListProvider);
    scrollPosition.jumpTo(tasks.isNotEmpty ? taskIndexToPosition(tasks.length - 1) : 0);
  }

  Future<void> _stopTask(int viewId) async {
    bool inOverview = ref.read(taskSwitcherState).inOverview;
    final tasks = ref.read(taskListProvider);
    final notifier = ref.read(taskSwitcherState.notifier);

    if (inOverview) {
      ref.read(taskStateProvider(viewId).notifier)
        ..open = false
        ..startDismissAnimation();
      return;
    }

    int closingTask = tasks.indexOf(viewId);
    int? focusingTask = taskToFocusAfterClosing(closingTask);
    // Might be null if there's no task left, in which case there's nothing to animate.
    if (focusingTask != null) {
      var currentTaskIndex = positionToTaskIndex(position);

      // Don't let the user interact with the task switcher while the animation is ongoing.
      notifier.disableUserControl = true;
      if (currentTaskIndex == closingTask) {
        await switchToTaskByIndex(focusingTask, zoomOut: true);
      } else {
        await switchToTaskByIndex(currentTaskIndex);
      }
      notifier.disableUserControl = false;

      _destroyTask(viewId);

      if (focusingTask > closingTask) {
        _updateContentDimensions(minScrollExtent: scrollPosition.minScrollExtent + _taskToTaskOffset);
      } else {
        _updateContentDimensions();
      }
    } else {
      _destroyTask(viewId);
      _updateContentDimensions();
    }

    // Just to ensure proper positioning in case the animations glitch out.
    _repositionTasks();
  }

  void _destroyTask(int viewId) {
    final textureId = ref.read(baseViewState(viewId)).textureId;
    PlatformApi.unregisterViewTexture(textureId);

    ref.read(taskListProvider.notifier).remove(viewId);
    ref.read(closingTaskListProvider.notifier).remove(viewId);

    ref.invalidate(baseViewState(viewId));
    ref.invalidate(windowState(viewId));
    ref.invalidate(windowWidget(viewId));
    ref.invalidate(taskPositionProvider(viewId));
    ref.invalidate(taskWidgetProvider(viewId));
    ref.invalidate(taskStateProvider(viewId));
  }

  int? taskToFocusAfterClosing(int closingTaskIndex) {
    if (ref.read(taskListProvider).length <= 1) {
      return null;
    } else if (closingTaskIndex == 0) {
      return 1;
    } else {
      return closingTaskIndex - 1;
    }
  }

  void _removeTask(int viewId) {
    ref.read(taskListProvider.notifier).remove(viewId);
    ref.read(closingTaskListProvider.notifier).add(viewId);
  }

  void stopAnimations() {
    _scaleAnimationController?.stop();
    _scaleAnimationController?.dispose();
    _scaleAnimationController = null;
  }

  Future<void> _switchToTask(int viewId) {
    return switchToTaskByIndex(ref.read(taskListProvider).indexOf(viewId));
  }

  Future<void> switchToTaskByIndex(int index, {bool zoomOut = false}) async {
    final viewId = ref.read(taskListProvider)[index];

    ref.read(taskSwitcherState.notifier)
      ..inOverview = false
      ..clearVisibleTasks()
      ..makeTaskVisible(viewId);

    PlatformApi.activateWindow(viewId);

    stopAnimations();
    _scaleAnimationController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    );

    final positionAnimationFuture = scrollPosition.animateTo(
      taskIndexToPosition(index),
      duration: const Duration(milliseconds: 400),
      curve: Curves.easeOutCubic,
    );

    final scale = ref.read(taskSwitcherState).scale;

    Animatable<double> scaleAnimatable;
    if (zoomOut) {
      scaleAnimatable = TweenSequence([
        TweenSequenceItem(
          weight: 30,
          tween: Tween(begin: scale, end: 0.8),
        ),
        TweenSequenceItem(
          weight: 70,
          tween: Tween(begin: 0.8, end: 1.0).chain(CurveTween(curve: Curves.easeOut)),
        ),
      ]);
    } else {
      scaleAnimatable = Tween(
        begin: scale,
        end: 1.0,
      ).chain(CurveTween(curve: Curves.easeOutCubic));
    }

    final notifier = ref.read(taskSwitcherState.notifier);

    final scaleAnimation = _scaleAnimationController!.drive(scaleAnimatable);
    scaleAnimation.addListener(() => notifier.scale = scaleAnimation.value);

    final scaleAnimationFuture = _scaleAnimationController!.forward().orCancel.catchError((_) => null);

    notifier.areAnimationsPlaying = true;
    await Future.wait([positionAnimationFuture, scaleAnimationFuture]);
    notifier.areAnimationsPlaying = false;
  }

  Future<void> showOverview() async {
    final notifier = ref.read(taskSwitcherState.notifier);
    notifier.inOverview = true;

    stopAnimations();
    _scaleAnimationController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    );

    final positionAnimationFuture = scrollPosition.animateTo(
      taskIndexToPosition(positionToTaskIndex(position)),
      duration: const Duration(milliseconds: 400),
      curve: Curves.easeOutCubic,
    );

    final scaleAnimation = _scaleAnimationController!.drive(
      Tween(
        begin: ref.read(taskSwitcherState).scale,
        end: 0.6,
      ).chain(CurveTween(curve: Curves.easeOutCubic)),
    );
    scaleAnimation.addListener(() => notifier.scale = scaleAnimation.value);

    final scaleAnimationFuture = _scaleAnimationController!.forward().orCancel.catchError((_) => null);

    notifier.areAnimationsPlaying = true;
    await Future.wait([positionAnimationFuture, scaleAnimationFuture]);
    notifier.areAnimationsPlaying = false;
  }

  @override
  void dispose() {
    scrollPosition.dispose();
    _scaleAnimationController?.dispose();
    for (var subscription in _streamSubscriptions) {
      subscription.cancel();
    }
    super.dispose();
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
