import 'package:flutter/material.dart';
import 'package:flutter_portal/flutter_portal.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/desktop/app_drawer/app_drawer.dart';
import 'package:zenith/ui/desktop/task_bar.dart';

final appDrawerVisibleProvider = StateProvider((ref) => false);

class AppDrawerButton extends ConsumerWidget {
  const AppDrawerButton({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return PortalTarget(
      visible: ref.watch(appDrawerVisibleProvider),
      portalFollower: Listener(
        behavior: HitTestBehavior.translucent,
        onPointerDown: (_) {
          ref.read(appDrawerVisibleProvider.notifier).state = false;
        },
      ),
      child: PortalTarget(
        visible: ref.watch(appDrawerVisibleProvider),
        portalFollower: const Padding(
          padding: EdgeInsets.only(bottom: 10),
          child: AppDrawer(),
        ),
        anchor: const Aligned(
          follower: Alignment.bottomCenter,
          target: Alignment.topCenter,
        ),
        child: PortalTarget(
          visible: ref.watch(appDrawerVisibleProvider),
          portalFollower: SizedBox(
            width: ref.watch(taskBarHeightProvider),
            height: ref.watch(taskBarHeightProvider),
            child: Listener(
              behavior: HitTestBehavior.opaque,
              onPointerDown: (_) => ref.read(appDrawerVisibleProvider.notifier).state = false,
            ),
          ),
          anchor: Aligned.center,
          child: SizedBox(
            height: double.infinity,
            child: IconButton(
              iconSize: 30,
              icon: const Icon(Icons.apps),
              onPressed: () => ref.read(appDrawerVisibleProvider.notifier).update((visible) => !visible),
            ),
          ),
        ),
      ),
    );
  }
}
