import 'package:elinux_app/window.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class DesktopState with ChangeNotifier {
  List<Window> windows = [];

  static const EventChannel windowMappedEvent = EventChannel('window_mapped');
  static const EventChannel windowUnmappedEvent = EventChannel('window_unmapped');
  static const MethodChannel platform = MethodChannel('platform');

  DesktopState() {
    windowMappedEvent.receiveBroadcastStream().listen((event) {
      int textureId = event["texture_id"];
      int viewPtr = event["view_ptr"];
      int width = event["width"];
      int height = event["height"];

      windows.add(Window(
        key: GlobalKey(),
        textureId: textureId,
        viewPtr: viewPtr,
        initialWidth: width,
        initialHeight: height,
      ));
      notifyListeners();
    });

    // Future.delayed(Duration(seconds: 1)).then((value) async {
    //   while (true) {
    //     await Future.delayed(Duration(seconds: 1));
    //     print("sec");
    //   }
    // });

    windowUnmappedEvent.receiveBroadcastStream().listen((event) {
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
    platform.invokeMethod('activate_window', window.viewPtr);
    notifyListeners();
  }

  void destroyWindow(Window window) {
    windows.remove(window);
    notifyListeners();
  }
}
