import 'package:elinux_app/window.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class DesktopState with ChangeNotifier {
  List<Window> windows = [
    Window(key: GlobalKey(), textureId: 0),
  ];

  static const EventChannel _newTextureIdEvent = EventChannel('new_texture_id');

  static const MethodChannel _test = MethodChannel('method test');

  DesktopState() {
    _newTextureIdEvent.receiveBroadcastStream().listen((event) {

      int textureId = event;
      print("MERGE $textureId");

      windows.add(Window(key: GlobalKey(), textureId: textureId));
      notifyListeners();
    });

    // Future.delayed(Duration(seconds: 1)).then((value) async {
    //   while (true) {
    //     await Future.delayed(Duration(seconds: 1));
    //     _test.invokeMethod("THIS_IS_A_METHOD", [1, 2, 3]);
    //   }
    // });
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
