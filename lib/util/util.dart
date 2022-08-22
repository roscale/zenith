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

class RectClipper extends CustomClipper<Rect> {
  Rect rect;

  RectClipper(this.rect);

  @override
  Rect getClip(Size size) {
    return rect;
  }

  @override
  bool shouldReclip(oldClipper) {
    return rect != (oldClipper as RectClipper).rect;
  }
}

extension RoundedExtension on Offset {
  Offset rounded() => Offset(dx.roundToDouble(), dy.roundToDouble());
}

extension RectClamp on Rect {
  Rect clampTo(Rect bigger) {
    double left = width > bigger.width ? 0 : this.left.clamp(bigger.left, bigger.right - width);
    double top = height > bigger.height ? 0 : this.top.clamp(bigger.top, bigger.bottom - height);
    return Rect.fromLTWH(left, top, width, height);
  }
}

/// Extends the clipping area with some padding.
class PaddedClipper extends CustomClipper<Rect> {
  EdgeInsets padding;

  PaddedClipper(this.padding);

  @override
  Rect getClip(Size size) {
    return Rect.fromLTRB(
      -padding.left,
      -padding.top,
      size.width + padding.right,
      size.height + padding.bottom,
    );
  }

  @override
  bool shouldReclip(oldClipper) {
    return padding != (oldClipper as PaddedClipper).padding;
  }
}
