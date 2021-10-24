import 'package:flutter/material.dart';

class WindowState with ChangeNotifier {
  WindowState(this._title, this._rect);

  String _title;
  Rect _rect;

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
}
