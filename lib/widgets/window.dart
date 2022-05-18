import 'dart:async';

import 'package:flutter/cupertino.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/desktop_state.dart';
import 'package:zenith/state/window_state.dart';

class Window extends StatelessWidget {
  final WindowState state;

  Window(this.state) : super(key: GlobalKey());

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => state,
      child: const _PointerListener(
        child: _Surface(),
      ),
    );
  }
}

class _PointerListener extends StatefulWidget {
  final Widget child;

  const _PointerListener({required this.child});

  @override
  State<_PointerListener> createState() => _PointerListenerState();
}

class _PointerListenerState extends State<_PointerListener> {
  late StreamSubscription pointerUpStreamSubscription;

  @override
  void initState() {
    super.initState();
    pointerUpStreamSubscription = context.read<DesktopState>().pointerUpStream.stream.listen((_event) {
      var windowState = context.read<WindowState>();
      // windowState.stopMove();
      // windowState.stopResize();
    });
  }

  @override
  void dispose() {
    pointerUpStreamSubscription.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    // var position = context.select((WindowState state) => state.position).rounded();
    var visibleBounds = context.select((WindowState state) => state.visibleBounds);
    var isClosing = context.select((WindowState state) => state.isClosing);

    return IgnorePointer(
      ignoring: isClosing,
      child: Listener(
        onPointerDown: (_) {
          var windowState = context.read<WindowState>();
          context.read<DesktopState>().activateWindow(windowState.viewId);
        },
        child: widget.child,
      ),
    );
  }
}

class _Surface extends StatelessWidget {
  const _Surface({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var windowState = context.read<WindowState>();
    var size = context.select((WindowState state) => state.surfaceSize);
    var bounds = context.select((WindowState state) => state.visibleBounds);
    var popups = context.select((WindowState state) => state.popups);

    return SizedBox(
      width: size.width,
      height: size.height,
      child: Stack(
        clipBehavior: Clip.none,
        children: [
          Listener(
            onPointerDown: (event) => pointerMoved(context, event),
            onPointerUp: (event) => pointerMoved(context, event),
            onPointerHover: (event) => pointerMoved(context, event),
            onPointerMove: (event) => pointerMoved(context, event),
            child: Texture(
              key: windowState.textureKey,
              filterQuality: FilterQuality.medium,
              textureId: windowState.viewId,
            ),
          ),
          ...popups,
        ],
      ),
    );
  }

  void pointerMoved(BuildContext context, PointerEvent event) {
    var windowState = context.read<WindowState>();

    if (true) {
      DesktopState.platform.invokeMethod(
        "pointer_hover",
        {
          "x": event.localPosition.dx,
          "y": event.localPosition.dy,
          "view_id": windowState.viewId,
        },
      );
    }
  }
}
