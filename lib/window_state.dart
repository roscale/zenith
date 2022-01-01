import 'dart:async';

import 'package:flutter/material.dart';

class WindowState with ChangeNotifier {
  WindowState({
    required this.viewId,
    required String title,
    required Offset position,
    required Size surfaceSize,
    required Rect visibleBounds,
  })  : _position = position,
        _title = title,
        _surfaceSize = surfaceSize,
        _visibleBounds = visibleBounds {
    WidgetsBinding.instance?.addPostFrameCallback((timeStamp) {
      // We cannot call this function directly because the window will not animate otherwise.
      animateOpening();
    });
  }

  final int viewId;
  final GlobalKey textureKey = GlobalKey();
  String _title;

  Offset _position;
  Size _surfaceSize;

  Rect _visibleBounds;
  Rect visibleBoundsResize = Rect.zero;

  bool isClosing = false;
  bool isMoving = false;
  bool isResizing = false;
  Offset movingDelta = Offset.zero;

  double scale = 0.9;
  double opacity = 0.5;
  var windowClosed = Completer<void>();

  // var popups = <Popup>[];

  String get title => _title;

  set title(String value) {
    _title = value;
    notifyListeners();
  }

  Offset get position => _position;

  set position(Offset value) {
    _position = value;
    notifyListeners();
  }

  Size get surfaceSize => _surfaceSize;

  set surfaceSize(Size surfaceSize) {
    _surfaceSize = surfaceSize;
    notifyListeners();
  }

  Rect get visibleBounds => _visibleBounds;

  set visibleBounds(Rect visibleBounds) {
    _visibleBounds = visibleBounds;
    notifyListeners();
  }

  void animateOpening() {
    scale = 1.0;
    opacity = 1.0;
    notifyListeners();
  }

  Future animateClosing() {
    isClosing = true;
    scale = 0.9;
    opacity = 0.0;
    notifyListeners();
    return windowClosed.future;
  }

  void startMove() {
    isMoving = true;
    movingDelta = Offset.zero;
    notifyListeners();
  }

  void stopMove() {
    if (isMoving) {
      isMoving = false;
      movingDelta = Offset.zero;
      notifyListeners();
    }
  }

  void startResize() {
    visibleBoundsResize = visibleBounds;
    isResizing = true;
    notifyListeners();
  }

  void stopResize() {
    if (isResizing) {
      isResizing = false;
      notifyListeners();
    }
  }
}
