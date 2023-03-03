import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:visibility_detector/visibility_detector.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/state/zenith_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/util/rect_overflow_box.dart';
import 'package:zenith/ui/common/popup.dart';
import 'package:zenith/ui/common/surface.dart';

final xdgToplevelSurfaceWidget = StateProvider.family<XdgToplevelSurface, int>((ref, int viewId) {
  return XdgToplevelSurface(key: ref.read(zenithXdgSurfaceStateProvider(viewId)).widgetKey, viewId: viewId);
});

// Overridden by the Window widget.
final _viewId = Provider<int>((ref) => throw UnimplementedError());

class XdgToplevelSurface extends ConsumerWidget {
  final int viewId;

  const XdgToplevelSurface({
    required Key key,
    required this.viewId,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return VisibilityDetector(
      key: ValueKey(viewId),
      onVisibilityChanged: (VisibilityInfo info) {
        bool visible = info.visibleFraction > 0;
        ref.read(zenithXdgToplevelStateProvider(viewId).notifier).visible = visible;
      },
      child: ProviderScope(
        overrides: [
          _viewId.overrideWithValue(viewId),
        ],
        child: _PointerListener(
          child: _Size(
            child: DeferredPointerHandler(
              child: Stack(
                clipBehavior: Clip.none,
                children: [
                  Surface(viewId: viewId),
                  // Consumer(builder: (_, WidgetRef ref, __) {
                  //   return ref.watch(surfaceWidget(ref.watch(_viewId)));
                  // }),
                  Consumer(
                    builder: (_, WidgetRef ref, __) {
                      List<int> popups = ref.watch(zenithXdgSurfaceStateProvider(viewId).select((v) => v.popups));
                      List<Widget> popupWidgets = popups.map((e) => ref.watch(popupWidget(e))).toList();

                      return Stack(
                        clipBehavior: Clip.none,
                        children: popupWidgets,
                      );
                    },
                  )
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class _PointerListener extends ConsumerWidget {
  final Widget child;

  const _PointerListener({
    Key? key,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Listener(
      onPointerDown: (_) => PlatformApi.activateWindow(ref.read(_viewId)),
      child: child,
    );
  }
}

class _Size extends ConsumerWidget {
  final Widget child;

  const _Size({
    Key? key,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final viewId = ref.watch(_viewId);

    return Consumer(
      builder: (_, WidgetRef ref, Widget? child) {
        Rect visibleBounds = ref.watch(zenithXdgSurfaceStateProvider(viewId)
            .select((v) => v.visibleBounds));

        return RectOverflowBox(
          rect: visibleBounds,
          child: child!,
        );
      },
      child: Consumer(
        builder: (_, WidgetRef ref, Widget? child) {
          Size surfaceSize = ref.watch(
              zenithSurfaceStateProvider(viewId).select((v) => v.surfaceSize));

          return SizedBox(
            width: surfaceSize.width,
            height: surfaceSize.height,
            child: child,
          );
        },
        child: child,
      ),
    );
  }
}
