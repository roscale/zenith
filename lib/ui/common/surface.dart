import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/zenith_subsurface_state.dart';
import 'package:zenith/ui/common/state/zenith_surface_state.dart';
import 'package:zenith/ui/common/subsurface.dart';
import 'package:zenith/ui/common/surface_size.dart';
import 'package:zenith/ui/common/view_input_listener.dart';

final surfaceWidget = StateProvider.family<Surface, int>((ref, int viewId) {
  return Surface(
    key: ref.read(zenithSurfaceStateProvider(viewId)).widgetKey,
    viewId: viewId,
  );
});

class Surface extends ConsumerWidget {
  final int viewId;

  const Surface({
    super.key,
    required this.viewId,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return SurfaceSize(
      viewId: viewId,
      child: Stack(
        clipBehavior: Clip.none,
        children: [
          Consumer(
            builder: (_, WidgetRef ref, __) {
              List<Widget> subsurfacesBelow = ref
                  .watch(zenithSurfaceStateProvider(viewId).select((v) => v.subsurfacesBelow))
                  .where((e) => ref.watch(zenithSubsurfaceStateProvider(e).select((v) => v.mapped)))
                  .map((e) => Subsurface(viewId: e))
                  .toList();

              return Stack(
                clipBehavior: Clip.none,
                children: subsurfacesBelow,
              );
            },
          ),
          ViewInputListener(
            viewId: viewId,
            child: Consumer(
              builder: (BuildContext context, WidgetRef ref, Widget? child) {
                Key key = ref.watch(zenithSurfaceStateProvider(viewId).select((v) => v.textureKey));
                int textureId = ref.watch(zenithSurfaceStateProvider(viewId).select((v) => v.textureId));

                return Texture(
                  key: key,
                  filterQuality: FilterQuality.medium,
                  textureId: textureId,
                );
              },
            ),
          ),
          Consumer(
            builder: (_, WidgetRef ref, __) {
              List<Widget> subsurfacesAbove = ref
                  .watch(zenithSurfaceStateProvider(viewId).select((v) => v.subsurfacesAbove))
                  .where((e) => ref.watch(zenithSubsurfaceStateProvider(e).select((v) => v.mapped)))
                  .map((e) => Subsurface(viewId: e))
                  .toList();

              return Stack(
                clipBehavior: Clip.none,
                children: subsurfacesAbove,
              );
            },
          ),
        ],
      ),
    );
  }
}
