import 'package:flutter/material.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/util/listenable_list.dart';
import 'package:zenith/widgets/popup.dart';

class WindowState {
  WindowState({
    required this.viewId,
    required Size surfaceSize,
    required Rect visibleBounds,
  })  : surfaceSize = ValueNotifier(surfaceSize),
        visibleBounds = ValueNotifier(visibleBounds);

  final int viewId;
  final GlobalKey textureKey = GlobalKey();
  bool _visible = true;

  final ValueNotifier<Size> surfaceSize;
  final ValueNotifier<Rect> visibleBounds;

  final popups = ListenableList<Popup>();

  void addPopup(Popup popup) {
    popup.state.parentViewId = viewId;
    popups.add(popup);
  }

  void removePopup(Popup popup) {
    popups.remove(popup);
  }

  void changeVisibility(bool visible) {
    if (_visible != visible) {
      _visible = visible;
      PlatformApi.changeWindowVisibility(viewId, visible);
    }
  }

  void dispose() {
    surfaceSize.dispose();
    visibleBounds.dispose();
    popups.dispose();
  }
}
