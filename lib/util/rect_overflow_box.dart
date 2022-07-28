import 'package:flutter/material.dart';

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
