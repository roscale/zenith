import 'dart:async';

import 'package:flutter/cupertino.dart';
import 'package:flutter/gestures.dart';
import 'package:zenith/util/clip_hitbox.dart';
import 'package:zenith/state/desktop_state.dart';
import 'package:zenith/enums.dart';
import 'package:zenith/util/util.dart';
import 'package:zenith/state/window_state.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class Window extends StatelessWidget {
  final WindowState state;

  Window(this.state) : super(key: GlobalKey());

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => state,
      child: const _PointerListener(
        child: _Animations(
          child: _Surface(),
        ),
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
      windowState.stopMove();
      windowState.stopResize();
    });
  }

  @override
  void dispose() {
    pointerUpStreamSubscription.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    var position = context.select((WindowState state) => state.position).rounded();
    var visibleBounds = context.select((WindowState state) => state.visibleBounds);
    var isClosing = context.select((WindowState state) => state.isClosing);
    var isMoving = context.select((WindowState state) => state.isMoving);
    var isResizing = context.select((WindowState state) => state.isResizing);

    return Positioned(
      left: position.dx - visibleBounds.left,
      top: position.dy - visibleBounds.top,
      child: IgnorePointer(
        ignoring: isClosing || isMoving || isResizing,
        child: Listener(
          onPointerDown: (_) {
            var windowState = context.read<WindowState>();
            windowState.accumulatedPointerDrag = Offset.zero;
            context.read<DesktopState>().activateWindow(windowState.viewId);
          },
          onPointerMove: (PointerMoveEvent event) {
            if (event.buttons & kPrimaryButton != 0) {
              // A move may be triggered late so keep track how much the user dragged the pointer.
              context.read<WindowState>().accumulatedPointerDrag += event.delta;
            }
            if (isMoving) {
              handleMove(context, event);
            }
            if (isResizing) {
              handleResize(context, event);
            }
          },
          child: widget.child,
        ),
      ),
    );
  }

  void handleMove(BuildContext context, PointerMoveEvent event) {
    var windowState = context.read<WindowState>();
    windowState.position += event.delta;
  }

  void handleResize(BuildContext context, PointerMoveEvent event) {
    var windowState = context.read<WindowState>();
    int edges = windowState.resizingEdges;

    double widthIncrement = 0;
    if (edges & Edges.right.id != 0) {
      widthIncrement = event.delta.dx;
    }
    if (edges & Edges.left.id != 0) {
      widthIncrement = -event.delta.dx;
    }

    double heightIncrement = 0;
    if (edges & Edges.bottom.id != 0) {
      heightIncrement = event.delta.dy;
    }
    if (edges & Edges.top.id != 0) {
      heightIncrement = -event.delta.dy;
    }

    windowState.wantedVisibleBounds = Rect.fromLTWH(
      windowState.wantedVisibleBounds.left,
      windowState.wantedVisibleBounds.top,
      windowState.wantedVisibleBounds.width + widthIncrement,
      windowState.wantedVisibleBounds.height + heightIncrement,
    );

    DesktopState.platform.invokeMethod(
      "resize_window",
      {
        "view_id": windowState.viewId,
        "width": windowState.wantedVisibleBounds.width,
        "height": windowState.wantedVisibleBounds.height,
      },
    );
  }
}

class _Animations extends StatelessWidget {
  final Widget child;

  const _Animations({required this.child});

  @override
  Widget build(BuildContext context) {
    var opacity = context.select((WindowState state) => state.opacity);
    var scale = context.select((WindowState state) => state.scale);

    return AnimatedOpacity(
      curve: Curves.easeOutCubic,
      opacity: opacity,
      duration: const Duration(milliseconds: 200),
      child: AnimatedScale(
        curve: Curves.easeOutCubic,
        scale: scale,
        duration: const Duration(milliseconds: 200),
        onEnd: () {
          var windowState = context.read<WindowState>();
          if (windowState.isClosing) {
            // This check is necessary because onEnd is also called when the window opening
            // animation ended.
            windowState.windowClosedCompleter.complete();
          }
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
    var windowState = context.read<WindowState>();
    var size = context.select((WindowState state) => state.surfaceSize);
    var bounds = context.select((WindowState state) => state.visibleBounds);
    const invisibleResizeBorder = 10.0;

    return ClipHitbox(
      clipper: RectClip(bounds.inflate(invisibleResizeBorder)),
      child: ClipRRect(
        borderRadius: const BorderRadius.all(Radius.circular(10)),
        child: SizedBox(
          width: size.width,
          height: size.height,
          child: Listener(
            onPointerDown: (event) => pointerMoved(context, event),
            onPointerUp: (event) => pointerMoved(context, event),
            onPointerHover: (event) => pointerMoved(context, event),
            onPointerMove: (event) => pointerMoved(context, event),
            child: Texture(
              key: windowState.textureKey,
              filterQuality: FilterQuality.none,
              textureId: windowState.viewId,
            ),
          ),
        ),
      ),
    );
  }

  void pointerMoved(BuildContext context, PointerEvent event) {
    var windowState = context.read<WindowState>();

    if (!windowState.isMoving && !windowState.isResizing) {
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
