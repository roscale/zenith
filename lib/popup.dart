import 'package:zenith/desktop_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';

class Popup extends StatelessWidget {
  final int x;
  final int y;
  final int width;
  final int height;
  final int viewId;
  final int parentSurfacePtr;
  final int surfacePtr;
  final GlobalKey frameGlobalKey = GlobalKey();

  Popup({
    required this.x,
    required this.y,
    required this.width,
    required this.height,
    required this.viewId,
    required this.parentSurfacePtr,
    required this.surfacePtr,
  }) : super(key: GlobalKey());

  @override
  Widget build(BuildContext context) {
    return Positioned(
      left: x.toDouble(),
      top: y.toDouble(),
      child: SizedBox(
        width: width.toDouble(),
        height: height.toDouble(),
        child: Listener(
          onPointerDown: pointerMoved,
          onPointerUp: pointerMoved,
          onPointerHover: pointerMoved,
          onPointerMove: pointerMoved,
          child: Texture(
            key: frameGlobalKey,
            textureId: viewId,
          ),
        ),
      ),
    );
  }

  void pointerMoved(PointerEvent event) {
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
