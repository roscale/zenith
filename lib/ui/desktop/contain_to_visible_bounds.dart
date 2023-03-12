import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/util/rect_overflow_box.dart';

class ContainToVisibleBounds extends ConsumerWidget {
  final int viewId;
  final Widget child;

  const ContainToVisibleBounds({
    super.key,
    required this.viewId,
    required this.child,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    Rect visibleBounds = ref.watch(zenithXdgSurfaceStateProvider(viewId).select((value) => value.visibleBounds));

    return RectOverflowBox(
      rect: visibleBounds,
      child: child,
    );
  }
}
