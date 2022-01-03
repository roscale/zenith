import 'package:flutter/material.dart';

extension GlobalKeyExtension on GlobalKey {
  Rect? get globalPaintBounds {
    final renderObject = currentContext?.findRenderObject();
    final translation = renderObject?.getTransformTo(null).getTranslation();
    if (translation != null && renderObject?.paintBounds != null) {
      final offset = Offset(translation.x, translation.y);
      return renderObject!.paintBounds.shift(offset);
    } else {
      return null;
    }
  }
}

class IdentityClip extends CustomClipper<Rect> {
  @override
  Rect getClip(Size size) {
    return Rect.fromLTWH(0, 0, size.width, size.height);
  }

  @override
  bool shouldReclip(oldClipper) {
    return false;
  }
}

class RectClip extends CustomClipper<Rect> {
  Rect rect;

  RectClip(this.rect);

  @override
  Rect getClip(Size size) {
    return rect;
  }

  @override
  bool shouldReclip(oldClipper) {
    return rect != (oldClipper as RectClip).rect;
  }
}

extension RoundedExtension on Offset {
  Offset rounded() => Offset(dx.roundToDouble(), dy.roundToDouble());
}
