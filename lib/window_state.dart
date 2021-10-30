import 'package:flutter/material.dart';

class WindowState with ChangeNotifier {
  WindowState(this._title, this._rect, this._textureId);

  String _title;
  Rect _rect;
  int _textureId;

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
}
