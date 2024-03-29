import 'package:flutter/material.dart';

// Crops the child to a rectangle inside it.
// The size of this widget will be the size of the rectangle.
// The child can still visually overflow even though hit-testing doesn't work outside the rectangle.
class RectOverflowBox extends StatelessWidget {
  final Widget child;
  final Rect rect;

  const RectOverflowBox({
    Key? key,
    required this.rect,
    required this.child,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return SizedOverflowBox(
      alignment: Alignment.topLeft,
      size: rect.size,
      child: Transform.translate(
        offset: -Offset(rect.left, rect.top),
        child: child,
      ),
    );
  }
}
