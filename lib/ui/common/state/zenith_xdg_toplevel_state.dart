import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/platform_api.dart';

part 'zenith_xdg_toplevel_state.freezed.dart';

final zenithXdgToplevelStateProvider = StateNotifierProvider.family<
    ZenithXdgToplevelStateNotifier,
    ZenithXdgToplevelState,
    int>((ref, int viewId) {
  return ZenithXdgToplevelStateNotifier(ref, viewId);
});

@freezed
class ZenithXdgToplevelState with _$ZenithXdgToplevelState {
  const factory ZenithXdgToplevelState({
    required bool visible,
    required Key virtualKeyboardKey,
    required FocusNode focusNode,
    required Object interactiveMoveRequested,
    required ResizeEdgeObject interactiveResizeRequested,
    required ToplevelDecoration decoration,
    required String title,
    required String appId,
  }) = _ZenithXdgToplevelState;
}

class ZenithXdgToplevelStateNotifier
    extends StateNotifier<ZenithXdgToplevelState> {
  final Ref _ref;
  final int _viewId;

  ZenithXdgToplevelStateNotifier(this._ref, this._viewId)
      : super(
          ZenithXdgToplevelState(
            visible: true,
            virtualKeyboardKey: GlobalKey(),
            focusNode: FocusNode(),
            interactiveMoveRequested: Object(),
            interactiveResizeRequested: ResizeEdgeObject(ResizeEdge.top),
            decoration: ToplevelDecoration.none,
            title: "",
            appId: "",
          ),
        );

  set visible(bool value) {
    if (value != state.visible) {
      PlatformApi.changeWindowVisibility(_viewId, value);
      state = state.copyWith(visible: value);
    }
  }

  void maximize(bool value) {
    PlatformApi.maximizeWindow(_viewId, value);
  }

  void resize(int width, int height) {
    PlatformApi.resizeWindow(_viewId, width, height);
  }

  void requestInteractiveMove() {
    state = state.copyWith(
      interactiveMoveRequested: Object(),
    );
  }

  void requestInteractiveResize(ResizeEdge edge) {
    state = state.copyWith(
      interactiveResizeRequested: ResizeEdgeObject(edge),
    );
  }

  void setDecoration(ToplevelDecoration decoration) {
    state = state.copyWith(
      decoration: decoration,
    );
  }

  void setTitle(String title) {
    state = state.copyWith(
      title: title,
    );
  }

  void setAppId(String appId) {
    state = state.copyWith(
      appId: appId,
    );
  }

  @override
  void dispose() {
    state.focusNode.dispose();
    super.dispose();
  }
}

enum ResizeEdge {
  topLeft,
  top,
  topRight,
  right,
  bottomRight,
  bottom,
  bottomLeft,
  left;

  static ResizeEdge fromInt(int n) {
    switch (n) {
      case 1:
        return top;
      case 2:
        return bottom;
      case 4:
        return left;
      case 5:
        return topLeft;
      case 6:
        return bottomLeft;
      case 8:
        return right;
      case 9:
        return topRight;
      case 10:
        return bottomRight;
      default:
        return bottomRight;
    }
  }
}

class ResizeEdgeObject {
  final ResizeEdge edge;

  ResizeEdgeObject(this.edge);
}

enum ToplevelDecoration {
  none,
  clientSide,
  serverSide;

  static ToplevelDecoration fromInt(int n) {
    switch (n) {
      case 0:
        return none;
      case 1:
        return clientSide;
      case 2:
        return serverSide;
      default:
        return none;
    }
  }
}
