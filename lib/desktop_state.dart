import 'package:elinux_app/window.dart';
import 'package:flutter/material.dart';

class DesktopState with ChangeNotifier {
  List<Window> windows = [
    Window(key: GlobalKey()),
    Window(key: GlobalKey()),
    Window(key: GlobalKey()),
  ];

  void activateWindow(Window window) {
    var index = windows.indexOf(window);
    windows.removeAt(index);
    windows.add(window);
    notifyListeners();
  }

  void destroyWindow(Window window) {
    windows.remove(window);
    notifyListeners();
  }
}
