import 'dart:async';

import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/mobile/state/task_state.dart';
import 'package:zenith/ui/mobile/state/task_switcher_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/ui/common/xdg_toplevel_surface.dart';
import 'package:zenith/ui/mobile/task_switcher/fitted_window.dart';
import 'package:zenith/ui/mobile/task_switcher/task_switcher.dart';
import 'package:zenith/ui/mobile/virtual_keyboard/with_virtual_keyboard.dart';

class Task extends ConsumerStatefulWidget {
  final int viewId;
  final VoidCallback onTap;

  const Task({
    Key? key,
    required this.viewId,
    required this.onTap,
  }) : super(key: key);

  @override
  ConsumerState<Task> createState() => _TaskState();
}

class _TaskState extends ConsumerState<Task> with SingleTickerProviderStateMixin {
  late final animationController = AnimationController(
    duration: const Duration(milliseconds: 250),
    vsync: this,
  );

  late var verticalPositionAnimation = Tween(begin: 0.0, end: 0.0).animate(animationController);

  @override
  void initState() {
    super.initState();
    animationController.addListener(() {
      ref.read(taskVerticalPositionProvider(widget.viewId).notifier).state = verticalPositionAnimation.value;
    });
  }

  @override
  void dispose() {
    animationController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    _registerListeners();

    return Consumer(
      builder: (_, WidgetRef ref, Widget? child) {
        final position = ref.watch(taskPositionProvider(widget.viewId));
        final constraints = ref.watch(taskSwitcherStateProvider.select((v) => v.constraints));

        double taskSwitcherPosition = ref.watch(taskSwitcherPositionProvider);
        double scale = ref.watch(taskSwitcherStateProvider.select((v) => v.scale));

        return Positioned(
          left: scale * (-taskSwitcherPosition + position),
          width: constraints.maxWidth,
          height: constraints.maxHeight,
          child: Transform.scale(
            scale: scale,
            child: child!,
          ),
        );
      },
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
            final inOverview = ref.watch(taskSwitcherStateProvider.select((v) => v.inOverview));
            final dismissState = ref.watch(taskStateProvider(widget.viewId).select((v) => v.dismissState));

            // Doing my best to not change the depth of the tree to avoid rebuilding the whole subtree.
            return IgnorePointer(
              ignoring: dismissState != TaskDismissState.notDismissed,
              child: GestureDetector(
                behavior: HitTestBehavior.opaque,
                onTap: inOverview ? _onTap : null,
                onVerticalDragDown: inOverview ? _onVerticalDragDown : null,
                onVerticalDragUpdate: inOverview ? _onVerticalDragUpdate : null,
                onVerticalDragEnd: inOverview ? _onVerticalDragEnd : null,
                onVerticalDragCancel: inOverview ? _onVerticalDragCancel : null,
                child: IgnorePointer(
                  ignoring: inOverview,
                  child: DeferredPointerHandler(
                    // Don't let the nested DeferPointer bypass the IgnorePointer.
                    child: child!,
                  ),
                ),
              ),
            );
          },
          child: Consumer(
            builder: (_, WidgetRef ref, Widget? child) {
              final virtualKeyboardKey =
                  ref.watch(zenithXdgToplevelStateProvider(widget.viewId).select((v) => v.virtualKeyboardKey));
              return WithVirtualKeyboard(
                key: virtualKeyboardKey,
                viewId: widget.viewId,
                child: child!,
              );
            },
            child: Consumer(
              builder: (_, WidgetRef ref, __) {
                final window = ref.watch(xdgToplevelSurfaceWidget(widget.viewId));
                return FittedWindow(
                  alignment: Alignment.topCenter,
                  window: window,
                );
              },
            ),
          ),
        ),
      ),
    );
  }

  void _registerListeners() {
    // Start dismiss animation.
    ref.listen(taskStateProvider(widget.viewId).select((value) => value.startDismissAnimation), (_, __) async {
      final notifier = ref.read(taskStateProvider(widget.viewId).notifier);
      TaskDismissState dismissState = notifier.state.dismissState;

      if (dismissState != TaskDismissState.notDismissed) {
        return;
      }
      notifier.dismissState = TaskDismissState.dismissing;

      // If the task is not destroyed after some time, the dismiss is cancelled.
      Future.delayed(const Duration(milliseconds: 500)).then((_) {
        if (mounted) {
          ref.read(taskStateProvider(widget.viewId).notifier).cancelDismissAnimation();
        }
      });

      await _startDismissAnimation();
      if (mounted) {
        notifier.dismissState = TaskDismissState.dismissed;
      }
    });

    // Cancel dismiss animation.
    ref.listen(taskStateProvider(widget.viewId).select((value) => value.cancelDismissAnimation), (_, __) {
      final notifier = ref.read(taskStateProvider(widget.viewId).notifier);
      notifier.dismissState = TaskDismissState.notDismissed;
      _cancelDismissAnimation();
    });
  }

  void _onTap() => widget.onTap();

  void _onVerticalDragDown(DragDownDetails _) => animationController.stop();

  void _onVerticalDragUpdate(DragUpdateDetails details) => ref
      .read(taskVerticalPositionProvider(widget.viewId).notifier)
      .update((state) => (state + details.primaryDelta!).clamp(-double.infinity, 0));

  void _onVerticalDragEnd(DragEndDetails details) {
    if (details.primaryVelocity! < -1000) {
      PlatformApi.closeView(widget.viewId);
      ref.read(taskStateProvider(widget.viewId).notifier).startDismissAnimation();
    } else {
      ref.read(taskStateProvider(widget.viewId).notifier).cancelDismissAnimation();
    }
  }

  void _onVerticalDragCancel() {
    if (ref.read(taskSwitcherStateProvider).inOverview) {
      ref.read(taskStateProvider(widget.viewId).notifier).cancelDismissAnimation();
    }
  }

  Future<void> _startDismissAnimation() {
    verticalPositionAnimation = Tween(
      begin: ref.read(taskVerticalPositionProvider(widget.viewId)),
      end: -1000.0,
    ).animate(CurvedAnimation(parent: animationController, curve: Curves.easeOutCubic));

    return animationController.forward(from: 0);
  }

  Future<void> _cancelDismissAnimation() {
    verticalPositionAnimation = Tween(
      begin: ref.read(taskVerticalPositionProvider(widget.viewId)),
      end: 0.0,
    ).animate(CurvedAnimation(parent: animationController, curve: Curves.easeOutCubic));

    return animationController.forward(from: 0);
  }
}
