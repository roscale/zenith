import 'package:fast_immutable_collections/fast_immutable_collections.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:riverpod_annotation/riverpod_annotation.dart';
import 'package:zenith/ui/common/state/surface_state.dart';
import 'package:zenith/ui/common/state/xdg_surface_state.dart';
import 'package:zenith/ui/common/state/xdg_toplevel_state.dart';
import 'package:zenith/ui/desktop/decorations/with_decorations.dart';
import 'package:zenith/ui/desktop/interactive_move_and_resize_listener.dart';
import 'package:zenith/ui/desktop/state/cursor_position_provider.dart';
import 'package:zenith/ui/desktop/state/window_move_provider.dart';
import 'package:zenith/ui/desktop/state/window_position_provider.dart';
import 'package:zenith/ui/desktop/state/window_resize_provider.dart';
import 'package:zenith/ui/desktop/state/window_stack_provider.dart';
import 'package:zenith/ui/mobile/state/virtual_keyboard_state.dart';

part '../../generated/ui/desktop/window.g.dart';

@Riverpod(keepAlive: true)
Window windowWidget(WindowWidgetRef ref, int viewId) {
  return Window(
    key: GlobalKey(),
    viewId: viewId,
  );
}

class Window extends ConsumerStatefulWidget {
  final int viewId;

  const Window({
    super.key,
    required this.viewId,
  });

  @override
  ConsumerState<Window> createState() => _WindowState();
}

class _WindowState extends ConsumerState<Window> with SingleTickerProviderStateMixin {
  late var controller = AnimationController(vsync: this, duration: const Duration(milliseconds: 200));
  late var scaleAnimation = Tween(begin: 0.8, end: 1.0).animate(
    CurvedAnimation(
      parent: controller,
      curve: Curves.easeOutCubic,
      reverseCurve: Curves.easeInCubic,
    ),
  );
  late var opacityAnimation = Tween(begin: 0.0, end: 1.0).animate(
    CurvedAnimation(
      parent: controller,
      curve: Curves.easeOutCubic,
      reverseCurve: Curves.easeInCubic,
    ),
  );

  @override
  void initState() {
    super.initState();
    spawnWindowAtCursor();
    controller.forward();
  }

  @override
  void dispose() {
    controller.dispose();
    super.dispose();
  }

  void spawnWindowAtCursor() {
    Future.microtask(() {
      Offset center = ref.read(cursorPositionProvider);
      Size surfaceSize = ref.read(surfaceStatesProvider(widget.viewId)).surfaceSize;
      Rect windowRect = Rect.fromCenter(
        center: center,
        width: surfaceSize.width,
        height: surfaceSize.height,
      );

      Size desktopSize = ref.read(windowStackProvider).desktopSize;// Offset windowPosition = ref.read(windowPositionProvider(widget.viewId));
      Rect desktopRect = Offset.zero & desktopSize;

      double dx = windowRect.right.clamp(desktopRect.left, desktopRect.right) - windowRect.right;
      double dy = windowRect.bottom.clamp(desktopRect.top, desktopRect.bottom) - windowRect.bottom;
      windowRect = windowRect.translate(dx, dy);

      dx = windowRect.left.clamp(desktopRect.left, desktopRect.right) - windowRect.left;
      dy = windowRect.top.clamp(desktopRect.top, desktopRect.bottom) - windowRect.top;
      windowRect = windowRect.translate(dx, dy);

      ref.read(windowPositionProvider(widget.viewId).notifier).state = windowRect.topLeft;
    });
  }

  @override
  Widget build(BuildContext context) {
    ref.listen(xdgSurfaceStatesProvider(widget.viewId).select((v) => v.visibleBounds), (Rect? previous, Rect next) {
      if (previous != null) {
        Offset offset =
            ref.read(windowResizeProvider(widget.viewId).notifier).computeWindowOffset(previous.size, next.size);
        ref.read(windowPositionProvider(widget.viewId).notifier).update((state) => state + offset);
      }
    });

    ref.listen(windowMoveProvider(widget.viewId).select((v) => v.movedPosition), (_, Offset position) {
      ref.read(windowPositionProvider(widget.viewId).notifier).state = position;
    });

    ref.listen(windowStackProvider.select((v) => v.animateClosing), (ISet<int>? previous, ISet<int> next) async {
      previous ??= ISet();
      if (!previous.contains(widget.viewId) && next.contains(widget.viewId)) {
        await controller.reverse().orCancel;
        ref.read(windowStackProvider.notifier).remove(widget.viewId);
      }
    });

    // Initialize the provider because it listens to events from other providers.
    ref.read(virtualKeyboardStateNotifierProvider(widget.viewId));

    return Consumer(
      builder: (BuildContext context, WidgetRef ref, Widget? child) {
        final offset = ref.watch(windowPositionProvider(widget.viewId));
        return Positioned(
          left: offset.dx,
          top: offset.dy,
          child: IgnorePointer(
            ignoring: ref.watch(windowStackProvider.select((v) => v.animateClosing)).contains(widget.viewId),
            child: child!,
          ),
        );
      },
      child: AnimatedBuilder(
        animation: scaleAnimation,
        builder: (BuildContext context, Widget? child) {
          return AnimatedBuilder(
            animation: opacityAnimation,
            builder: (BuildContext context, Widget? child) {
              return Opacity(
                opacity: opacityAnimation.value,
                child: Transform.scale(
                  scale: scaleAnimation.value,
                  transformHitTests: false,
                  filterQuality: FilterQuality.none,
                  child: child,
                ),
              );
            },
            child: child,
          );
        },
        child: WithDecorations(
          viewId: widget.viewId,
          child: InteractiveMoveAndResizeListener(
            viewId: widget.viewId,
            child: Consumer(
              builder: (BuildContext context, WidgetRef ref, Widget? child) {
                return ref.watch(xdgToplevelSurfaceWidgetProvider(widget.viewId));
              },
            ),
          ),
        ),
      ),
    );
  }
}
