import 'package:provider/provider.dart';
import 'package:zenith/desktop_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:zenith/popup_state.dart';
import 'package:zenith/util.dart';

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
    var state = context.watch<PopupState>();

    return Positioned(
      left: (state.position.dx - state.visibleBounds.left).toDouble(),
      top: (state.position.dy - state.visibleBounds.top).toDouble(),
      child: IgnorePointer(
        child: child,
        ignoring: state.isClosing,
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
    return ClipRect(
      clipper: IdentityClip(),
      child: FadeTransition(
        opacity: _fadeAnimation,
        child: SlideTransition(
          transformHitTests: false,
          position: _offsetAnimation,
          child: widget.child,
        ),
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

    return SizedBox(
      width: state.surfaceSize.width,
      height: state.surfaceSize.height,
      child: Listener(
        onPointerDown: (event) => pointerMoved(event, state.viewId),
        onPointerUp: (event) => pointerMoved(event, state.viewId),
        onPointerHover: (event) => pointerMoved(event, state.viewId),
        onPointerMove: (event) => pointerMoved(event, state.viewId),
        child: Texture(
          key: state.textureKey,
          textureId: state.viewId,
        ),
      ),
    );
  }

  void pointerMoved(PointerEvent event, int viewId) {
    DesktopState.platform.invokeMethod(
      "pointer_hover",
      {
        "x": event.localPosition.dx,
        "y": event.localPosition.dy,
        "view_id": viewId,
      },
    );
  }
}
