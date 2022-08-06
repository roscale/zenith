import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/popup_state.dart';
import 'package:zenith/util/multi_value_listenable_builder.dart';
import 'package:zenith/widgets/view_input_listener.dart';

class Popup extends StatelessWidget {
  final PopupState state;

  Popup(this.state) : super(key: state.widgetKey);

  @override
  Widget build(BuildContext context) {
    return Provider(
      create: (_) => state,
      dispose: (_, PopupState state) => state.dispose(),
      child: _Positioner(
        child: _Animations(
          key: state.animationsKey,
          child: _Surface(),
        ),
      ),
    );
  }
}

class _Positioner extends StatelessWidget {
  final Widget child;

  const _Positioner({Key? key, required this.child}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var state = context.read<PopupState>();

    return ValueListenableBuilder2(
      first: state.position,
      second: state.visibleBounds,
      builder: (_, Offset position, Rect visibleBounds, Widget? child) {
        return Positioned(
          left: position.dx - visibleBounds.left,
          top: position.dy - visibleBounds.top,
          child: child!,
        );
      },
      child: ValueListenableBuilder(
        valueListenable: state.isClosing,
        builder: (_, bool isClosing, Widget? child) {
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

class _Animations extends StatefulWidget {
  final Widget child;

  @override
  State<_Animations> createState() => AnimationsState();

  const _Animations({Key? key, required this.child}) : super(key: key);
}

class AnimationsState extends State<_Animations> with SingleTickerProviderStateMixin {
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
    begin: Offset(0.0, -10 / context.read<PopupState>().surfaceSize.value.height),
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

class _Surface extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    var state = context.read<PopupState>();

    return ValueListenableBuilder(
      valueListenable: state.surfaceSize,
      builder: (_, Size size, Widget? child) {
        return SizedBox(
          width: size.width,
          height: size.height,
          child: child,
        );
      },
      child: DeferPointer(
        child: DeferredPointerHandler(
          child: Stack(
            clipBehavior: Clip.none,
            children: [
              ViewInputListener(
                viewId: state.viewId,
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
          ),
        ),
      ),
    );
  }
}
