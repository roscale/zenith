import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/xdg_toplevel_surface.dart';
import 'package:zenith/ui/desktop/window_manager.dart';

final windowPositionStateProvider = StateProvider.family<Offset, int>((ref, int viewId) => Offset.zero);

final windowWidget = StateProvider.family<Window, int>((ref, int viewId) {
  return Window(
    key: GlobalKey(),
    viewId: viewId,
  );
});

class Window extends ConsumerWidget {
  final int viewId;

  const Window({
    super.key,
    required this.viewId,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Consumer(
      builder: (BuildContext context, WidgetRef ref, Widget? child) {
        final offset = ref.watch(windowPositionStateProvider(viewId));
        return Positioned(
          left: offset.dx,
          top: offset.dy,
          child: child!,
        );
      },
      child: Listener(
        onPointerDown: (_) {
          PlatformApi.activateWindow(viewId);
          ref.read(windowStackNotifierProvider.notifier).raise(viewId);
        },
        child: IntrinsicWidth(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              TitleBar(viewId: viewId),
              ref.watch(xdgToplevelSurfaceWidget(viewId)),
            ],
          ),
        ),
      ),
    );
  }
}

class TitleBar extends ConsumerStatefulWidget {
  final int viewId;

  const TitleBar({super.key, required this.viewId});

  @override
  ConsumerState<ConsumerStatefulWidget> createState() {
    return _TitleBarState();
  }
}

class _TitleBarState extends ConsumerState<TitleBar> {
  var startPosition = Offset.zero;
  var delta = Offset.zero;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onPanDown: (DragDownDetails details) {
        startPosition = ref.read(windowPositionStateProvider(widget.viewId));
        delta = Offset.zero;
      },
      onPanUpdate: (DragUpdateDetails details) {
        delta += details.delta;
        var offset = startPosition + delta;
        offset = Offset(offset.dx.roundToDouble(), offset.dy.roundToDouble());
        ref.read(windowPositionStateProvider(widget.viewId).notifier).state = offset;
      },
      child: SizedBox(
        height: 30,
        child: Material(
          color: Colors.white70,
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              SizedBox(
                width: 30,
                height: 30,
                child: IconButton(
                  padding: EdgeInsets.zero,
                  splashRadius: 22,
                  iconSize: 18,
                  icon: const Icon(Icons.close),
                  onPressed: () {
                    PlatformApi.closeView(widget.viewId);
                  },
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
