import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:zenith/util/listenable_list.dart';
import 'package:zenith/widgets/popup.dart';

class PopupState {
  PopupState({
    required this.viewId,
    required Offset position,
    required Size surfaceSize,
    required Rect visibleBounds,
  })  : position = ValueNotifier(position),
        visibleBounds = ValueNotifier(visibleBounds),
        surfaceSize = ValueNotifier(surfaceSize);

  final int viewId;
  late int parentViewId;
  final textureKey = GlobalKey();
  final animationsKey = GlobalKey<AnimationsState>();
  final popups = ListenableList<Popup>();

  final ValueNotifier<Offset> position;
  final ValueNotifier<Size> surfaceSize;
  final ValueNotifier<Rect> visibleBounds;

  final _isClosing = ValueNotifier(false);

  ValueListenable<bool> get isClosing => _isClosing;

  FutureOr animateClosing() {
    _isClosing.value = true;
    return animationsKey.currentState?.controller.reverse();
  }

  void addPopup(Popup popup) {
    popup.state.parentViewId = viewId;
    popups.add(popup);
  }

  void removePopup(Popup popup) {
    popups.remove(popup);
  }

  void dispose() {
    position.dispose();
    surfaceSize.dispose();
    visibleBounds.dispose();
    _isClosing.dispose();
    popups.dispose();
  }
}
