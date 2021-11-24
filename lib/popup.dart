import 'package:zenith/desktop_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';

class Popup extends StatelessWidget {
  final int x;
  final int y;
  final int width;
  final int height;
  final int textureId;
  final int viewPtr;
  final int parentSurfacePtr;
  final int surfacePtr;
  final GlobalKey frameGlobalKey = GlobalKey();

  Popup({
    required this.x,
    required this.y,
    required this.width,
    required this.height,
    required this.textureId,
    required this.viewPtr,
    required this.parentSurfacePtr,
    required this.surfacePtr,
  }) : super(key: GlobalKey());

  @override
  Widget build(BuildContext context) {
    print("build");
    return Positioned(
      left: x.toDouble(),
      top: y.toDouble(),
      child: SizedBox(
        width: width.toDouble(),
        height: height.toDouble(),
        // child: Container(color: Colors.red),
        child: Listener(
          onPointerHover: (PointerHoverEvent event) => pointerMoved(event),
          onPointerMove: (PointerMoveEvent event) => pointerMoved(event),
          child: Texture(
            key: frameGlobalKey,
            textureId: textureId,
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
        "view_ptr": viewPtr,
      },
    );
  }
}
