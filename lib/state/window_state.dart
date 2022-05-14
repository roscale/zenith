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
    WidgetsBinding.instance.addPostFrameCallback((_timeStamp) {
      // We cannot call this function directly because the window will not animate otherwise.
      // It must be called after the frame the widget is constructed.
      _animateOpening();
    });
  }

  final int viewId;
  final GlobalKey textureKey = GlobalKey();

  // Completes when the window is no longer visible.
  final windowClosedCompleter = Completer();

  // Some apps don't trigger an interactive move as soon as the pointer drags the window.
  // GTK apps in particular will wait for the pointer to move a few pixels while the left mouse button is
  // pressed, before engaging an interactive move. This variable tracks how much the user dragged the pointer before officially moving
  // the window, and it's applied to the window position as soon as the move begins, so that the window won't lag behind the pointer.
  Offset accumulatedPointerDrag = Offset.zero;

  // While resizing, we keep track of the window size the user wants, and it is passed to the application. The application
  // might choose to resize to another size than the one indicated. Gnome Terminal does this where it likes to resize to
  // increments of a character, so the resizing isn't smooth. This variable is just indicative to let the application know
  // what the user wants and may not represent the actual window size.
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
    if (_isMoving) {
      return;
    }
    _isMoving = true;
    // Apply any accumulated pointer drag if the move started late.
    _position += accumulatedPointerDrag;
    notifyListeners();
  }

  void stopMove() {
    _isMoving = false;
    accumulatedPointerDrag = Offset.zero;
    notifyListeners();
  }

  void startResize(int edges) {
    if (_isResizing) {
      return;
    }
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
