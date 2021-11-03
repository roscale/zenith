import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

class CustomStack extends Stack {
  CustomStack({Key? key, children}) : super(key: key, children: children);

  @override
  CustomRenderStack createRenderObject(BuildContext context) {
    return CustomRenderStack(
      alignment: alignment,
      textDirection: textDirection ?? Directionality.of(context),
      fit: fit,
      clipBehavior: clipBehavior,
    );
  }
}

class CustomRenderStack extends RenderStack {
  CustomRenderStack({alignment, textDirection, fit, clipBehavior})
      : super(
            alignment: alignment,
            textDirection: textDirection,
            fit: fit,
            clipBehavior: clipBehavior);

  @override
  bool hitTestChildren(BoxHitTestResult result, {required Offset position}) {
    var stackHits = 0;

    final children = getChildrenAsList();

    for (var child in children.reversed) {
      final StackParentData? childParentData =
          child.parentData as StackParentData?;

      final childHit = result.addWithPaintOffset(
        offset: childParentData!.offset,
        position: position,
        hitTest: (BoxHitTestResult result, Offset transformed) {
          assert(transformed == position - childParentData.offset);
          return child.hitTest(result, position: transformed);
        },
      );
      if (childHit) {
        stackHits += 1;
      }

      if (stackHits == 2) {
        break;
      }
    }

    return stackHits > 0;
  }
}
