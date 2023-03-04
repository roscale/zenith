import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/popup_stack.dart';
import 'package:zenith/ui/desktop/state/window_stack_notifier_provider.dart';
import 'package:zenith/ui/desktop/window.dart';

final windowStackNotifierProvider =
    StateNotifierProvider<WindowStackNotifierProvider, WindowStack>((ref) => WindowStackNotifierProvider());

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

    var windowStackNotifier = ref.read(windowStackNotifierProvider.notifier);
    var tasks = ref.read(mappedWindowListProvider);

    Future.microtask(() {
      windowStackNotifier.set(tasks);
    });

    ref.listenManual(windowMappedStreamProvider, (_, AsyncValue<int> next) {
      next.whenData((int viewId) {
        windowStackNotifier.add(viewId);
      });
    });

    ref.listenManual(windowUnmappedStreamProvider, (_, AsyncValue<int> next) {
      next.whenData((int viewId) {
        windowStackNotifier.remove(viewId);
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    final tasks = ref.watch(windowStackNotifierProvider).windows;

    return Stack(
      key: ref.watch(windowStackGlobalKey),
      children: [
        for (int viewId in tasks) ref.watch(windowWidget(viewId)),
        const PopupStack(),
      ],
    );
  }
}
