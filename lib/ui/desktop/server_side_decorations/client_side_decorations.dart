import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/zenith_surface_state.dart';
import 'package:zenith/util/rect_overflow_box.dart';

class ClientSideDecorations extends StatelessWidget {
  final int viewId;
  final Widget child;

  const ClientSideDecorations({
    super.key,
    required this.viewId,
    required this.child,
  });

  @override
  Widget build(BuildContext context) {
    return _InputRegion(
      viewId: viewId,
      child: child,
    );
  }
}

class _InputRegion extends ConsumerWidget {
  final int viewId;
  final Widget child;

  const _InputRegion({
    super.key,
    required this.viewId,
    required this.child,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    Rect inputRegion = ref.watch(zenithSurfaceStateProvider(viewId).select((v) => v.inputRegion));

    return RectOverflowBox(
      rect: inputRegion,
      child: child,
    );
  }
}
