import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/state/app_drawer_state.dart';

class AppDrawerHandle extends ConsumerWidget {
  const AppDrawerHandle({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return GestureDetector(
      onPanStart: (DragStartDetails details) {
        Overlay.of(context)?.insert(ref.read(appDrawerStateProvider).overlayEntry);
        ref.read(appDrawerStateProvider.notifier).update((state) => state.copyWith(
              overlayEntryInserted: true,
              dragging: true,
              offset: state.slideDistance,
            ));
      },
      onPanEnd: (DragEndDetails details) {
        ref.read(appDrawerStateProvider.notifier).update(
              (state) => state.copyWith(
                dragging: false,
                dragVelocity: details.velocity.pixelsPerSecond.dy,
              ),
            );
      },
      onPanUpdate: (DragUpdateDetails details) {
        ref.read(appDrawerStateProvider.notifier).update(
              (state) => state.copyWith(
                offset: (state.offset + details.delta.dy).clamp(0, state.slideDistance),
              ),
            );
      },
      child: Container(
        color: Colors.black.withOpacity(0.5),
        height: 100,
      ),
    );
  }
}
