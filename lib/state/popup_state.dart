import 'dart:async';

import 'package:flutter/material.dart';
import 'package:zenith/widgets/popup.dart';

class PopupState with ChangeNotifier {
  PopupState({
    required this.viewId,
    required Offset position,
    required Size surfaceSize,
    required Rect visibleBounds,
  })  : _position = position,
        _visibleBounds = visibleBounds,
        _surfaceSize = surfaceSize;

  final int viewId;
  late int parentViewId;
  final GlobalKey textureKey = GlobalKey();
  final GlobalKey<AnimationsState> animationsKey = GlobalKey();
  final List<Popup> _popups = [];

  Offset _position;
  Size _surfaceSize;
  Rect _visibleBounds;

  bool _isClosing = false;

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

  FutureOr animateClosing() {
    _isClosing = true;
    notifyListeners();
    return animationsKey.currentState?.controller.reverse();
  }

  List<Popup> get popups => List.unmodifiable(_popups);

  void addPopup(Popup popup) {
    _popups.add(popup);
    popup.state.parentViewId = viewId;
    notifyListeners();
  }

  void removePopup(Popup popup) {
    _popups.remove(popup);
    notifyListeners();
  }
}
