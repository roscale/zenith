import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:visibility_detector/visibility_detector.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/util/rect_overflow_box.dart';
import 'package:zenith/widgets/view_input_listener.dart';

class Window extends StatelessWidget {
  final WindowState state;

  Window(this.state) : super(key: state.widgetKey);

  @override
  Widget build(BuildContext context) {
    return Provider(
      create: (_) => state,
      dispose: (_, WindowState state) => state.dispose(),
      child: const _PointerListener(
        child: _Size(
          child: _Surface(),
        ),
      ),
    );
  }
}

class _PointerListener extends StatelessWidget {
  final Widget child;

  const _PointerListener({
    Key? key,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Listener(
      onPointerDown: (_) {
        var state = context.read<WindowState>();
        PlatformApi.activateWindow(state.viewId);
      },
      child: child,
    );
  }
}

class _Size extends StatelessWidget {
  final Widget child;

  const _Size({
    Key? key,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var state = context.read<WindowState>();

    return ValueListenableBuilder(
      valueListenable: state.visibleBounds,
      builder: (_, Rect visibleBounds, Widget? child) {
        return RectOverflowBox(
          rect: visibleBounds,
          child: child!,
        );
      },
      child: ValueListenableBuilder(
        valueListenable: state.surfaceSize,
        builder: (_, Size surfaceSize, Widget? child) {
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

class _Surface extends StatelessWidget {
  const _Surface({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var state = context.read<WindowState>();

    return Stack(
      clipBehavior: Clip.none,
      children: [
        ViewInputListener(
          viewId: state.viewId,
          child: VisibilityDetector(
            key: ValueKey(state.viewId),
            onVisibilityChanged: (VisibilityInfo info) {
              bool visible = !info.visibleBounds.isEmpty;
              state.changeVisibility(visible);
            },
            child: ValueListenableBuilder(
              valueListenable: state.textureId,
              builder: (BuildContext context, int textureId, Widget? child) {
                return Texture(
                  key: state.textureKey,
                  filterQuality: FilterQuality.medium,
                  textureId: textureId,
                );
              },
            ),
          ),
        ),
        AnimatedBuilder(
          animation: state.popups,
          builder: (_, __) {
            return Stack(
              clipBehavior: Clip.none,
              children: state.popups,
            );
          },
        ),
      ],
    );
  }
}
