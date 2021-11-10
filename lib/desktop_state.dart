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

      if (windows.isNotEmpty) {
        windows.last.getWindowState().deactivate();
      }

      windows.add(
        Window(
          textureId: textureId,
          viewPtr: viewPtr,
          initialWidth: width,
          initialHeight: height,
        ),
      );
      notifyListeners();
    });

    // Future.delayed(Duration(seconds: 1)).then((value) async {
    //   while (true) {
    //     await Future.delayed(Duration(seconds: 1));
    //     print("sec");
    //   }
    // });

    windowUnmappedEvent.receiveBroadcastStream().listen((event) async {
      int textureId = event["texture_id"];
      print("before $textureId");
      // try {
      //   windows.removeLast();
      // } on Exception catch(e) {
      //   print("WRONG");
      // }
      var window = windows.singleWhere((element) => element.textureId == textureId);
      await window.getWindowState().close();
      windows.remove(window);
      notifyListeners();
      print("after");
    });
  }

  void activateWindow(Window window) {
    // Deactivate current active window.
    if (windows.isNotEmpty) {
      windows.last.getWindowState().deactivate();
    }

    var index = windows.indexOf(window);
    windows.removeAt(index);
    windows.add(window);
    window.getWindowState().activate();

    platform.invokeMethod('activate_window', window.viewPtr);
    notifyListeners();
  }

  void destroyWindow(Window window) {
    windows.remove(window);
    notifyListeners();
  }
}
