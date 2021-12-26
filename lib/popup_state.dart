import 'dart:async';

import 'package:flutter/material.dart';

class PopupState with ChangeNotifier {
  PopupState({
    required this.viewId,
    required this.parentViewId,
    required Offset position,
    required this.surfaceSize,
    required this.visibleBounds,
  }) : _position = position {
    WidgetsBinding.instance?.addPostFrameCallback((timeStamp) {
      // We cannot call this function directly because the window will not animate otherwise.
      animateOpening();
    });
  }

  final int viewId;
  final int parentViewId;
  final GlobalKey textureKey = GlobalKey();

  Offset _position;
  Size surfaceSize;
  Rect visibleBounds;

  double scale = 0.9;
  double opacity = 0.5;
  var closed = Completer<void>();

  Offset get position => _position;

  set position(Offset value) {
    _position = value;
    notifyListeners();
  }

  void animateOpening() {
    scale = 1.0;
    opacity = 1.0;
    notifyListeners();
  }

  Future animateClosing() {
    scale = 0.9;
    opacity = 0.0;
    notifyListeners();
    return closed.future;
  }
}
