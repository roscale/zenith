import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/base_view_state.dart';
import 'package:zenith/util/rect_overflow_box.dart';
import 'package:zenith/widgets/popup.dart';
import 'package:zenith/widgets/view_input_listener.dart';

final windowWidget = StateProvider.family<Window, int>((ref, int viewId) {
  return const Window(key: Key(""), viewId: -1);
});

// Overridden by the Window widget.
final _viewId = Provider<int>((ref) => throw UnimplementedError());

class Window extends StatelessWidget {
  final int viewId;

  const Window({
    required Key key,
    required this.viewId,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return ProviderScope(
      overrides: [
        _viewId.overrideWithValue(viewId),
      ],
      child: const _PointerListener(
        child: _Size(
          child: _Surface(),
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
        Rect visibleBounds = ref.watch(baseViewState(viewId).select((v) => v.visibleBounds));
        return RectOverflowBox(
          rect: visibleBounds,
          child: child!,
        );
      },
      child: Consumer(
        builder: (_, WidgetRef ref, Widget? child) {
          Size surfaceSize = ref.watch(baseViewState(viewId).select((v) => v.surfaceSize));
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

class _Surface extends ConsumerWidget {
  const _Surface({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final viewId = ref.watch(_viewId);

    return Stack(
      clipBehavior: Clip.none,
      children: [
        ViewInputListener(
          viewId: viewId,
          child: Consumer(
            builder: (BuildContext context, WidgetRef ref, Widget? child) {
              Key textureKey = ref.watch(baseViewState(viewId).select((v) => v.textureKey));
              int textureId = ref.watch(baseViewState(viewId).select((v) => v.textureId));
              return Texture(
                key: textureKey,
                filterQuality: FilterQuality.medium,
                textureId: textureId,
              );
            },
          ),
        ),
        Consumer(
          builder: (_, WidgetRef ref, __) {
            List<int> popups = ref.watch(baseViewState(viewId).select((v) => v.popups));
            List<Widget> popupWidgets = popups.map((e) => ref.watch(popupWidget(e))).toList();

            return Stack(
              clipBehavior: Clip.none,
              children: popupWidgets,
            );
          },
        ),
      ],
    );
  }
}
