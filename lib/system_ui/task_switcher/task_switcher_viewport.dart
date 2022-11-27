import 'package:flutter/material.dart';
import 'package:flutter/src/widgets/framework.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/state/task_switcher_state.dart';

class TaskSwitcherViewport extends ConsumerWidget {
  final ScrollPosition scrollPosition;
  final Widget child;

  const TaskSwitcherViewport({
    Key? key,
    required this.scrollPosition,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Consumer(
      builder: (_, WidgetRef ref, __) {
        double scale = ref.watch(taskSwitcherStateProvider.select((v) => v.scale));
        return AnimatedBuilder(
          animation: scrollPosition,
          builder: (_, __) => Transform(
            filterQuality: FilterQuality.high,
            alignment: Alignment.center,
            transform: Matrix4.identity()
              ..scale(scale)
              ..translate(-scrollPosition.pixels),
            child: child,
          ),
        );
      },
    );
  }
}
