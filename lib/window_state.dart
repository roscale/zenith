import 'dart:async';

import 'package:flutter/material.dart';

class WindowState with ChangeNotifier {
  WindowState(this._title, this._rect, this._textureId);

  String _title;
  Rect _rect;
  int _textureId;

  bool isClosing = false;
  double scale = 0.9;
  double opacity = 0.5;
  double shadowBlurRadius = 10;
  var windowClosed = Completer<void>();

  String get title => _title;

  set title(String value) {
    _title = value;
    notifyListeners();
  }

  Rect get rect => _rect;

  set rect(Rect value) {
    _rect = value;
    notifyListeners();
  }

  int get textureId => _textureId;

  set textureId(int value) {
    _textureId = value;
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
  
  void activate() {
    shadowBlurRadius = 30;
    notifyListeners();
  }
  
  void deactivate() {
    shadowBlurRadius = 10;
    notifyListeners();
  }
}
