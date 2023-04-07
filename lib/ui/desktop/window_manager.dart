import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/popup_stack.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/ui/desktop/state/window_stack_provider.dart';
import 'package:zenith/ui/desktop/window.dart';

final windowStackGlobalKey = Provider((ref) => GlobalKey());

class WindowManager extends ConsumerStatefulWidget {
  const WindowManager({super.key});

  @override
  ConsumerState<WindowManager> createState() => _WindowManagerState();
}

class _WindowManagerState extends ConsumerState<WindowManager> {
  @override
  void initState() {
    super.initState();

    var windowStackNotifier = ref.read(windowStackProvider.notifier);
    var tasks = ref.read(mappedWindowListProvider);

    Future.microtask(() {
      windowStackNotifier.set(tasks);
    });

    ref.listenManual(windowMappedStreamProvider, (_, AsyncValue<int> next) {
      next.whenData((int viewId) {
        windowStackNotifier.add(viewId);
        ref.read(zenithXdgToplevelStateProvider(viewId)).focusNode.requestFocus();
      });
    });

    ref.listenManual(windowUnmappedStreamProvider, (_, AsyncValue<int> next) {
      next.whenData((int viewId) {
        windowStackNotifier.remove(viewId);
      });
    });

    PlatformApi.startWindowsMaximized(false);
  }

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (BuildContext context, BoxConstraints constraints) {
        PlatformApi.maximizedWindowSize(constraints.maxWidth.toInt(), constraints.maxHeight.toInt());

        return Consumer(
          builder: (BuildContext context, WidgetRef ref, Widget? child) {
            final tasks = ref.watch(windowStackProvider).windows;

            return Stack(
              clipBehavior: Clip.none,
              key: ref.watch(windowStackGlobalKey),
              children: [
                for (int viewId in tasks) ref.watch(windowWidget(viewId)),
                const PopupStack(),
              ],
            );
          },
        );
      },
    );
  }
}
