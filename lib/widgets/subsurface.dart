import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/state/zenith_subsurface_state.dart';
import 'package:zenith/widgets/surface.dart';

final subsurfaceWidget = StateProvider.family<Subsurface, int>((ref, int viewId) {
  return const Subsurface(key: Key(""), viewId: -1);
});

// Overridden by the widget.
final _viewId = Provider<int>((ref) => throw UnimplementedError());

class Subsurface extends StatelessWidget {
  final int viewId;

  const Subsurface({
    super.key,
    required this.viewId,
  });

  @override
  Widget build(BuildContext context) {
    return ProviderScope(
      overrides: [
        _viewId.overrideWithValue(viewId),
      ],
      child: _Positioner(
        child: Consumer(
          builder: (BuildContext context, WidgetRef ref, Widget? child) {
            return Surface(viewId: viewId);
            // return ref.watch(surfaceWidget(viewId));
          },
        ),
      ),
    );
  }
}

class _Positioner extends ConsumerWidget {
  final Widget child;

  const _Positioner({Key? key, required this.child}) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    int viewId = ref.watch(_viewId);
    return Consumer(
      builder: (_, WidgetRef ref, Widget? child) {
        Offset position = ref.watch(zenithSubsurfaceStateProvider(viewId).select((v) => v.position));

        return Positioned(
          left: position.dx,
          top: position.dy,
          child: child!,
        );
      },
      child: child,
    );
  }
}
