import 'dart:async';

import 'package:flutter/material.dart';
import 'package:zenith/popup.dart';

class PopupState with ChangeNotifier {
  PopupState({
    required this.viewId,
    required this.parentViewId,
    required Offset position,
    required this.surfaceSize,
    required this.visibleBounds,
  }) : _position = position;

  final int viewId;
  final int parentViewId;
  final GlobalKey textureKey = GlobalKey();
  final GlobalKey<AnimationsState> animationsKey = GlobalKey();

  Offset _position;
  Size surfaceSize;
  Rect visibleBounds;

  bool isClosing = false;

  Offset get position => _position;

  set position(Offset value) {
    _position = value;
    notifyListeners();
  }

  FutureOr animateClosing() {
    isClosing = true;
    notifyListeners();
    return animationsKey.currentState?.controller.reverse();
  }
}
