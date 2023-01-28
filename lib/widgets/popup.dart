import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/state/zenith_surface_state.dart';
import 'package:zenith/state/zenith_xdg_popup_state.dart';
import 'package:zenith/state/zenith_xdg_surface_state.dart';
import 'package:zenith/widgets/surface.dart';
import 'package:zenith/widgets/surface_size.dart';

final popupWidget = StateProvider.family<Popup, int>((ref, int viewId) {
  return const Popup(key: Key(""), viewId: -1);
});

final _viewId = Provider<int>((ref) => throw UnimplementedError());

class Popup extends StatelessWidget {
  final int viewId;

  const Popup({
    required Key key,
    required this.viewId,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return ProviderScope(
      overrides: [
        _viewId.overrideWithValue(viewId),
      ],
      child: _Positioner(
        child: Consumer(
          builder: (BuildContext context, WidgetRef ref, Widget? child) {
            int viewId = ref.watch(_viewId);
            Key key = ref.watch(zenithXdgPopupStateProvider(viewId).select((v) => v.animationsKey));
            return _Animations(
              key: key,
              child: child!,
            );
          },
          child: DeferPointer(
            // Allow hit testing for nested popups
            child: DeferredPointerHandler(
              child: SurfaceSize(
                // Need to specify the surface size, because the IgnorePointer parent
                // needs a size and this stack doesn't have a size otherwise.
                viewId: viewId,
                child: Stack(
                  clipBehavior: Clip.none,
                  children: [
                    Surface(viewId: viewId),
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
        Offset position = ref.watch(zenithXdgPopupStateProvider(viewId).select((v) => v.position));
        int parentId = ref.watch(zenithXdgPopupStateProvider(viewId).select((v) => v.parentViewId));
        Rect parentVisibleBounds = ref.watch(zenithXdgSurfaceStateProvider(parentId).select((v) => v.visibleBounds));
        Rect visibleBounds = ref.watch(zenithXdgSurfaceStateProvider(viewId).select((v) => v.visibleBounds));

        return Positioned(
          left: position.dx - visibleBounds.left + parentVisibleBounds.left,
          top: position.dy - visibleBounds.top + parentVisibleBounds.top,
          child: child!,
        );
      },
      child: Consumer(
        builder: (_, WidgetRef ref, Widget? child) {
          bool isClosing = ref.watch(zenithXdgPopupStateProvider(viewId).select((v) => v.isClosing));
          return IgnorePointer(
            ignoring: isClosing,
            child: child,
          );
        },
        child: child,
      ),
    );
  }
}

class _Animations extends ConsumerStatefulWidget {
  final Widget child;

  @override
  ConsumerState<_Animations> createState() => AnimationsState();

  const _Animations({Key? key, required this.child}) : super(key: key);
}

class AnimationsState extends ConsumerState<_Animations> with SingleTickerProviderStateMixin {
  late final viewId = ref.read(_viewId);

  @override
  Widget build(BuildContext context) {
    return FadeTransition(
      opacity: _fadeAnimation,
      child: SlideTransition(
        transformHitTests: false,
        position: _offsetAnimation,
        child: widget.child,
      ),
    );
  }

  late final AnimationController controller = AnimationController(
    duration: const Duration(milliseconds: 200),
    reverseDuration: const Duration(milliseconds: 100),
    vsync: this,
  )..forward();

  late final Animation<Offset> _offsetAnimation = Tween<Offset>(
    begin: Offset(
      0.0,
      -10.0 / ref.read(zenithSurfaceStateProvider(viewId)).surfaceSize.height,
    ),
    end: Offset.zero,
  ).animate(CurvedAnimation(
    parent: controller,
    curve: Curves.easeOutCubic,
  ));

  late final Animation<double> _fadeAnimation = Tween<double>(
    begin: 0.0,
    end: 1.0,
  ).animate(CurvedAnimation(
    parent: controller,
    curve: Curves.easeOutCubic,
  ));

  @override
  void dispose() {
    controller.dispose();
    super.dispose();
  }
}
