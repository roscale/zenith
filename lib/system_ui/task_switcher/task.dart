import 'dart:async';

import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:hooks_riverpod/hooks_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/task_state.dart';
import 'package:zenith/state/task_switcher_state.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/system_ui/task_switcher/fitted_window.dart';
import 'package:zenith/system_ui/task_switcher/task_switcher.dart';
import 'package:zenith/system_ui/virtual_keyboard/with_virtual_keyboard.dart';
import 'package:zenith/widgets/window.dart';

class Task extends ConsumerStatefulWidget {
  final int viewId;
  final VoidCallback onTap;
  final VoidCallback onClosed;

  const Task({
    Key? key,
    required this.viewId,
    required this.onTap,
    required this.onClosed,
  }) : super(key: key);

  @override
  ConsumerState<Task> createState() => _TaskState();
}

class _TaskState extends ConsumerState<Task> with SingleTickerProviderStateMixin {
  late final controller = AnimationController(
    duration: const Duration(milliseconds: 250),
    vsync: this,
  );

  late var animation = Tween(begin: 0.0, end: 0.0).animate(controller);

  @override
  void initState() {
    super.initState();
    controller.addListener(() {
      ref.read(taskVerticalPositionProvider(widget.viewId).notifier).state = animation.value;
    });
  }

  @override
  void dispose() {
    controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    ref.listen(taskStateProvider(widget.viewId).select((value) => value.startDismissAnimation), (_, __) async {
      final notifier = ref.read(taskStateProvider(widget.viewId).notifier);
      TaskDismissState dismissState = notifier.state.dismissState;
      if (dismissState != TaskDismissState.open) {
        return;
      }

      notifier.dismissState = TaskDismissState.dismissing;
      await _startSlideClosingAnimation();
      notifier.dismissState = TaskDismissState.dismissed;
    });

    ref.listen(taskStateProvider(widget.viewId).select((value) => value.cancelDismissAnimation), (_, __) {
      final notifier = ref.read(taskStateProvider(widget.viewId).notifier);
      notifier.dismissState = TaskDismissState.open;
      _animateBackToPosition();
    });

    ref.listen(taskStateProvider(widget.viewId).select((value) => value.dismissState),
        (_, TaskDismissState dismissState) async {
      if (dismissState == TaskDismissState.dismissing) {
        await Future.delayed(const Duration(milliseconds: 500));
        if (mounted) {
          ref.read(taskStateProvider(widget.viewId).notifier).cancelDismissAnimation();
        }
      }
    });

    return Consumer(
      builder: (_, WidgetRef ref, Widget? child) {
        final position = ref.watch(taskPositionProvider(widget.viewId));
        final constraints = ref.watch(taskSwitcherState.select((v) => v.constraints));

        return Positioned(
          left: position,
          width: constraints.maxWidth,
          height: constraints.maxHeight,
          child: child!,
        );
      },
      child: DeferPointer(
        child: Consumer(
          builder: (BuildContext context, WidgetRef ref, Widget? child) {
            double position = ref.watch(taskVerticalPositionProvider(widget.viewId));

            return Transform.translate(
              offset: Offset(0, position),
              child: Opacity(
                opacity: (1 - position.abs() / 1000).clamp(0, 1),
                child: child!,
              ),
            );
          },
          child: Consumer(
            builder: (_, WidgetRef ref, Widget? child) {
              final inOverview = ref.watch(taskSwitcherState.select((v) => v.inOverview));
              final dismissState = ref.watch(taskStateProvider(widget.viewId).select((v) => v.dismissState));

              // Doing my best to not change the depth of the tree to avoid rebuilding the whole subtree.
              return IgnorePointer(
                ignoring: dismissState != TaskDismissState.open,
                child: GestureDetector(
                  behavior: HitTestBehavior.opaque,
                  onTap: inOverview ? () => widget.onTap() : null,
                  onVerticalDragUpdate: inOverview
                      ? (DragUpdateDetails details) {
                          ref
                              .read(taskVerticalPositionProvider(widget.viewId).notifier)
                              .update((state) => (state + details.primaryDelta!).clamp(-double.infinity, 0));
                        }
                      : null,
                  onVerticalDragDown: inOverview ? (_) => controller.stop() : null,
                  onVerticalDragCancel:
                      inOverview ? ref.read(taskStateProvider(widget.viewId).notifier).cancelDismissAnimation : null,
                  onVerticalDragEnd: inOverview
                      ? (DragEndDetails details) async {
                          if (details.primaryVelocity! < -1000) {
                            PlatformApi.closeView(widget.viewId);
                            ref.read(taskStateProvider(widget.viewId).notifier).startDismissAnimation();
                          } else {
                            ref.read(taskStateProvider(widget.viewId).notifier).cancelDismissAnimation();
                          }
                        }
                      : null,
                  child: IgnorePointer(
                    ignoring: inOverview,
                    child: Listener(
                      onPointerDown: !inOverview ? (_) => _moveTaskToTheEnd(ref) : (_) {},
                      child: child,
                    ),
                  ),
                ),
              );
            },
            child: Consumer(
              builder: (_, WidgetRef ref, Widget? child) {
                final virtualKeyboardKey = ref.watch(windowState(widget.viewId).select((v) => v.virtualKeyboardKey));
                return WithVirtualKeyboard(
                  key: virtualKeyboardKey,
                  viewId: widget.viewId,
                  child: child!,
                );
              },
              child: Consumer(
                builder: (_, WidgetRef ref, __) {
                  final window = ref.watch(windowWidget(widget.viewId));
                  return FittedWindow(
                    alignment: Alignment.topCenter,
                    window: window,
                  );
                },
              ),
            ),
          ),
        ),
      ),
    );
  }

  void _moveTaskToTheEnd(WidgetRef ref) async {
    if (ref.read(taskListProvider).last == widget.viewId) {
      return;
    }
    final notifier = ref.read(taskSwitcherState.notifier);
    notifier.disableUserControl = true;
    // Move the task to the end after the animations have finished.
    await _untilAnimationsStopped(ref);
    // moveCurrentTaskToEnd();
    // jumpToLastTask();
    notifier.disableUserControl = false;
  }

  Future<void> _startSlideClosingAnimation() {
    animation = Tween(
      begin: ref.read(taskVerticalPositionProvider(widget.viewId)),
      end: -1000.0,
    ).animate(CurvedAnimation(parent: controller, curve: Curves.easeOutCubic));

    return controller.forward(from: 0);
  }

  Future<void> _animateBackToPosition() {
    animation = Tween(
      begin: ref.read(taskVerticalPositionProvider(widget.viewId)),
      end: 0.0,
    ).animate(CurvedAnimation(parent: controller, curve: Curves.easeOutCubic));

    return controller.forward(from: 0);
  }
}

Future<void> _untilAnimationsStopped(WidgetRef ref) {
  final completer = Completer<void>();

  if (!ref.read(taskSwitcherState).areAnimationsPlaying) {
    completer.complete();
  } else {
    late ProviderSubscription<bool> subscription;
    subscription = ref.listenManual(
      taskSwitcherState.select((v) => v.areAnimationsPlaying),
      (_, bool animationsPlaying) {
        if (!animationsPlaying) {
          subscription.close();
          completer.complete();
        }
      },
    );
  }
  return completer.future;
}
