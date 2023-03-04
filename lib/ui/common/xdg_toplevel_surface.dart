import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:visibility_detector/visibility_detector.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/ui/common/surface.dart';
import 'package:zenith/util/rect_overflow_box.dart';

final xdgToplevelSurfaceWidget = StateProvider.family<XdgToplevelSurface, int>((ref, int viewId) {
  return XdgToplevelSurface(key: ref.read(zenithXdgSurfaceStateProvider(viewId)).widgetKey, viewId: viewId);
});

class XdgToplevelSurface extends ConsumerWidget {
  final int viewId;

  const XdgToplevelSurface({
    required super.key,
    required this.viewId,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return VisibilityDetector(
      key: ValueKey(viewId),
      onVisibilityChanged: (VisibilityInfo info) {
        bool visible = info.visibleFraction > 0;
        if (ref.context.mounted) {
          ref.read(zenithXdgToplevelStateProvider(viewId).notifier).visible = visible;
        }
      },
      child: _PointerListener(
        viewId: viewId,
        child: _VisibleBounds(
          viewId: viewId,
          child: Surface(
            viewId: viewId,
          ),
        ),
      ),
    );
  }
}

class _PointerListener extends ConsumerWidget {
  final int viewId;
  final Widget child;

  const _PointerListener({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Listener(
      onPointerDown: (_) => PlatformApi.activateWindow(viewId),
      child: child,
    );
  }
}

class _VisibleBounds extends ConsumerWidget {
  final int viewId;
  final Widget child;

  const _VisibleBounds({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    Rect visibleBounds = ref.watch(zenithXdgSurfaceStateProvider(viewId).select((v) => v.visibleBounds));

    return RectOverflowBox(
      rect: visibleBounds,
      child: child,
    );
  }
}
