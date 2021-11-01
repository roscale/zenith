import 'package:elinux_app/window.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class DesktopState with ChangeNotifier {
  List<Window> windows = [
    // Window(key: GlobalKey(), textureId: 0),
  ];

  static const EventChannel _newTextureIdEvent = EventChannel('new_texture_id');

  DesktopState() {
    _newTextureIdEvent.receiveBroadcastStream().listen((event) {
      int textureId = event;
      windows.add(Window(key: GlobalKey(), textureId: textureId));
      notifyListeners();
    });
  }

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
