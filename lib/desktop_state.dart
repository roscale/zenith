import 'package:elinux_app/window.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class DesktopState with ChangeNotifier {
  List<Window> windows = [
    Window(textureId: 0, initialWidth: 300, initialHeight: 300)
  ];

  static const EventChannel _windowMappedEvent = EventChannel('window_mapped');
  static const EventChannel _windowUnmappedEvent =
      EventChannel('window_unmapped');

  DesktopState() {
    _windowMappedEvent.receiveBroadcastStream().listen((event) {
      int textureId = event["texture_id"];
      int width = event["width"];
      int height = event["height"];

      windows.add(Window(
        key: GlobalKey(),
        textureId: textureId,
        initialWidth: width,
        initialHeight: height,
      ));
      notifyListeners();
    });

    Future.delayed(Duration(seconds: 1)).then((value) async {
      while (true) {
        await Future.delayed(Duration(seconds: 1));
        print("sec");
      }
    });

    _windowUnmappedEvent.receiveBroadcastStream().listen((event) {
      int textureId = event["texture_id"];
      print("before $textureId");
      // try {
      //   windows.removeLast();
      // } on Exception catch(e) {
      //   print("WRONG");
      // }
      windows.removeWhere((element) => element.textureId == textureId);
      notifyListeners();
      print("after");
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
