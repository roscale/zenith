import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/zenith_subsurface_state.dart';
import 'package:zenith/ui/common/surface.dart';

final subsurfaceWidget = StateProvider.family<Subsurface, int>((ref, int viewId) {
  return Subsurface(
    key: ref.read(zenithSubsurfaceStateProvider(viewId)).widgetKey,
    viewId: viewId,
  );
});

class Subsurface extends StatelessWidget {
  final int viewId;

  const Subsurface({
    super.key,
    required this.viewId,
  });

  @override
  Widget build(BuildContext context) {
    return _Positioner(
      viewId: viewId,
      child: Consumer(
        builder: (BuildContext context, WidgetRef ref, Widget? child) {
          return Surface(viewId: viewId);
          // return ref.watch(surfaceWidget(viewId));
        },
      ),
    );
  }
}

class _Positioner extends ConsumerWidget {
  final int viewId;
  final Widget child;

  const _Positioner({
    Key? key,
    required this.viewId,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    Offset position = ref.watch(zenithSubsurfaceStateProvider(viewId).select((v) => v.position));

    return Positioned(
      left: position.dx,
      top: position.dy,
      child: child,
    );
  }
}
