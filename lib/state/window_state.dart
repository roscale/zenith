import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/widgets/popup.dart';

class WindowState with ChangeNotifier {
  WindowState({
    required this.viewId,
    required Size surfaceSize,
    required Rect visibleBounds,
  })  : _surfaceSize = ValueNotifier(surfaceSize),
        _visibleBounds = visibleBounds;

  final int viewId;
  final GlobalKey textureKey = GlobalKey();

  final ValueNotifier<Size> _surfaceSize;
  Rect _visibleBounds;
  final List<Popup> _popups = [];

  bool _visible = true;
  bool _isClosing = false;

  Size get surfaceSize => _surfaceSize.value;

  ValueListenable<Size> get surfaceSizeListenable => _surfaceSize;

  set surfaceSize(Size surfaceSize) {
    _surfaceSize.value = surfaceSize;
    notifyListeners();
  }

  Rect get visibleBounds => _visibleBounds;

  set visibleBounds(Rect visibleBounds) {
    _visibleBounds = visibleBounds;
    notifyListeners();
  }

  bool get isClosing => _isClosing;

  List<Popup> get popups => List.unmodifiable(_popups);

  void addPopup(Popup popup) {
    _popups.add(popup);
    popup.state.parentViewId = viewId;
    notifyListeners();
  }

  void removePopup(Popup popup) {
    _popups.remove(popup);
    notifyListeners();
  }

  void changeVisibility(bool visible) {
    if (_visible != visible) {
      _visible = visible;
      PlatformApi.changeWindowVisibility(viewId, visible);
    }
  }
}
