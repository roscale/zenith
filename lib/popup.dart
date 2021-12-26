import 'package:provider/provider.dart';
import 'package:zenith/desktop_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:zenith/popup_state.dart';
import 'package:zenith/util.dart';

class Popup extends StatefulWidget {
  final PopupState state;

  Popup(this.state) : super(key: GlobalKey());

  @override
  State<Popup> createState() => _PopupState();
}

class _PopupState extends State<Popup> with SingleTickerProviderStateMixin {
  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider.value(
      value: widget.state,
      child: _Positioner(
        child: _Animations(
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
      child: child,
    );
  }
}

class _Animations extends StatefulWidget {
  final Widget child;

  @override
  State<_Animations> createState() => _AnimationsState();

  const _Animations({Key? key, required this.child}) : super(key: key);
}

class _AnimationsState extends State<_Animations> with SingleTickerProviderStateMixin {
  late final AnimationController _controller = AnimationController(
    duration: const Duration(milliseconds: 200),
    vsync: this,
  )..forward();

  late final Animation<Offset> _offsetAnimation = Tween<Offset>(
    begin: Offset(0.0, -10 / context.read<PopupState>().surfaceSize.height),
    end: Offset.zero,
  ).animate(CurvedAnimation(
    parent: _controller,
    curve: Curves.easeOutCubic,
  ));

  late final Animation<double> _fadeAnimation = Tween<double>(
    begin: 0.0,
    end: 1.0,
  ).animate(CurvedAnimation(
    parent: _controller,
    curve: Curves.easeOutCubic,
  ));

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
