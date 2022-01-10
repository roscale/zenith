import 'dart:async';

import 'package:flutter/material.dart';

class WindowState with ChangeNotifier {
  WindowState({
    required this.viewId,
    required String title,
    required Offset position,
    required Size surfaceSize,
    required Rect visibleBounds,
  })  : _title = title,
        _position = position,
        _surfaceSize = surfaceSize,
        _visibleBounds = visibleBounds {
    WidgetsBinding.instance?.addPostFrameCallback((_timeStamp) {
      // We cannot call this function directly because the window will not animate otherwise.
      // It must be called after the frame the widget is constructed.
      _animateOpening();
    });
  }

  final int viewId;
  final GlobalKey textureKey = GlobalKey();

  // Completes when the window is no longer visible.
  final windowClosedCompleter = Completer();
  Rect wantedVisibleBounds = Rect.zero;

  String _title;
  Offset _position;
  Size _surfaceSize;
  Rect _visibleBounds;

  bool _isMoving = false;
  bool _isResizing = false;
  bool _isClosing = false;

  int _resizingEdges = 0;

  // Animation values.
  double _scale = 0.9;
  double _opacity = 0.5;

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

  bool get isClosing => _isClosing;

  bool get isMoving => _isMoving;

  bool get isResizing => _isResizing;

  int get resizingEdges => _resizingEdges;

  double get scale => _scale;

  double get opacity => _opacity;

  void _animateOpening() {
    _scale = 1.0;
    _opacity = 1.0;
    notifyListeners();
  }

  void startMove() {
    assert(!_isMoving);
    _isMoving = true;
    notifyListeners();
  }

  void stopMove() {
    _isMoving = false;
    notifyListeners();
  }

  void startResize(int edges) {
    assert(!_isResizing);
    _isResizing = true;
    wantedVisibleBounds = visibleBounds;
    _resizingEdges = edges;
    notifyListeners();
  }

  void stopResize() {
    _isResizing = false;
    notifyListeners();
  }

  Future animateClosing() {
    assert(!_isClosing);
    _isClosing = true;
    _scale = 0.9;
    _opacity = 0.0;
    notifyListeners();
    return windowClosedCompleter.future;
  }
}
