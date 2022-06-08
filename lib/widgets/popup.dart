import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/popup_state.dart';
import 'package:zenith/util/util.dart';
import 'package:zenith/widgets/view_input_listener.dart';

class Popup extends StatelessWidget {
  final PopupState state;

  Popup(this.state) : super(key: GlobalKey());

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => state,
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
    var position = context.select((PopupState state) => state.position).rounded();
    var visibleBounds = context.select((PopupState state) => state.visibleBounds);
    var isClosing = context.select((PopupState state) => state.isClosing);

    return Positioned(
      left: position.dx - visibleBounds.left,
      top: position.dy - visibleBounds.top,
      child: IgnorePointer(
        child: child,
        ignoring: isClosing,
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
    begin: Offset(0.0, -10 / context.read<PopupState>().surfaceSize.height),
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

    return Selector(
      selector: (_, PopupState state) => state.surfaceSize,
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
                child: Texture(
                  key: state.textureKey,
                  filterQuality: FilterQuality.medium,
                  textureId: state.viewId,
                ),
              ),
              Selector(
                selector: (_, PopupState popupState) => popupState.popups,
                builder: (_, List<Popup> popups, __) {
                  return Stack(
                    clipBehavior: Clip.none,
                    children: popups,
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
