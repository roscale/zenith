import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

class WindowStack extends Stack {
  WindowStack({Key? key, children}) : super(key: key, children: children);

  @override
  WindowRenderStack createRenderObject(BuildContext context) {
    return WindowRenderStack(
      alignment: alignment,
      textDirection: textDirection ?? Directionality.maybeOf(context),
      fit: fit,
      clipBehavior: overflow == Overflow.visible ? Clip.none : clipBehavior,
    );
  }
}

class WindowRenderStack extends RenderStack {
  WindowRenderStack({alignment, textDirection, fit, clipBehavior})
      : super(alignment: alignment, textDirection: textDirection, fit: fit, clipBehavior: clipBehavior);

  @override
  bool hitTestChildren(BoxHitTestResult result, {required Offset position}) {
    final children = getChildrenAsList();

    for (var child in children.reversed) {
      final StackParentData? childParentData = child.parentData as StackParentData?;

      final childHit = result.addWithPaintOffset(
        offset: childParentData!.offset,
        position: position,
        hitTest: (BoxHitTestResult result, Offset transformed) {
          assert(transformed == position - childParentData.offset);
          return child.hitTest(result, position: transformed);
        },
      );

      if (childHit) {
        return true;
      }
    }

    return false;
  }
}
