import 'package:zenith/popup.dart';
import 'package:zenith/util.dart';
import 'package:zenith/window.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class DesktopState with ChangeNotifier {
  List<Window> windows = [];
  List<Popup> popups = [];

  static const EventChannel windowMappedEvent = EventChannel('window_mapped');
  static const EventChannel windowUnmappedEvent = EventChannel('window_unmapped');
  static const EventChannel popupMappedEvent = EventChannel('popup_mapped');
  static const EventChannel popupUnmappedEvent = EventChannel('popup_unmapped');
  static const MethodChannel platform = MethodChannel('platform');

  DesktopState() {
    windowMappedEvent.receiveBroadcastStream().listen((event) {
      int viewId = event["view_id"];
      int surfacePtr = event["surface_ptr"];
      int width = event["width"];
      int height = event["height"];

      if (windows.isNotEmpty) {
        windows.last.getWindowState().deactivate();
      }

      windows.add(
        Window(
          viewId: viewId,
          surfacePtr: surfacePtr,
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
      int viewId = event["view_id"];
      print("before $viewId");
      // try {
      //   windows.removeLast();
      // } on Exception catch(e) {
      //   print("WRONG");
      // }
      var window = windows.singleWhere((element) => element.viewId == viewId);
      await window.getWindowState().animateClosing();
      windows.remove(window);
      notifyListeners();
      print("after ${windows.length}");
    });

    popupMappedEvent.receiveBroadcastStream().listen((event) {
      int viewId = event["view_id"];
      int surfacePtr = event["surface_ptr"];
      int parentSurfacePtr = event["parent_surface_ptr"];
      int x = event["x"];
      int y = event["y"];
      int width = event["width"];
      int height = event["height"];

      // Parent can be either a window or another popup.
      Rect rect;
      var windowIndex = windows.indexWhere((element) => element.surfacePtr == parentSurfacePtr);
      if (windowIndex != -1) {
        rect = windows[windowIndex].frameGlobalKey.globalPaintBounds!;
      } else {
        var popupIndex = popups.indexWhere((element) => element.surfacePtr == parentSurfacePtr);
        rect = popups[popupIndex].frameGlobalKey.globalPaintBounds!;
      }

      // var parentWindow = windows.singleWhere((element) => element.surfacePtr == parentSurfacePtr);

      var popup = Popup(
        x: x + rect.left.toInt(),
        y: y + rect.top.toInt(),
        width: width,
        height: height,
        viewId: viewId,
        parentSurfacePtr: parentSurfacePtr,
        surfacePtr: surfacePtr,
      );
      // var parentWindow = windows.singleWhere((element) => element.surfacePtr == parentSurfacePtr);
      popups.add(popup);
      notifyListeners();
      // parentWindow.getWindowState().popups.add(popup);
      // parentWindow.getWindowState().notifyListeners();
    });

    popupUnmappedEvent.receiveBroadcastStream().listen((event) {
      int viewId = event["view_id"];

      popups.removeWhere((element) => element.viewId == viewId);
      notifyListeners();
      // parentWindow.getWindowState().popups.removeWhere((popup) => popup.viewId == viewId);
      // parentWindow.getWindowState().notifyListeners();
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

    platform.invokeMethod('activate_window', window.viewId);
    notifyListeners();
  }

  void destroyWindow(Window window) {
    windows.remove(window);
    notifyListeners();
  }
}
