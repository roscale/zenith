import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/desktop/server_side_decorations/server_side_decorations.dart';

part 'resizing_state_notifier_provider.freezed.dart';

final resizingStateNotifierProvider =
    StateNotifierProvider.family<ResizingStateNotifierProvider, ResizerState, int>((ref, int viewId) {
  return ResizingStateNotifierProvider(viewId);
});

class ResizingStateNotifierProvider extends StateNotifier<ResizerState> {
  final int viewId;

  ResizingStateNotifierProvider(this.viewId)
      : super(
          const ResizerState(
            resizingSide: null,
            wantedSize: Size.zero,
          ),
        );

  void startResize(ResizingSide side, Size size) {
    state = ResizerState(
      resizingSide: side,
      wantedSize: size,
    );
  }

  void resize(Offset delta) {
    delta = _computeResizeOffset(delta);

    state = state.copyWith(
      wantedSize: state.wantedSize + delta,
    );

    int width = max(1, state.wantedSize.width.toInt());
    int height = max(1, state.wantedSize.height.toInt());

    PlatformApi.resizeWindow(
      viewId,
      width,
      height,
    );
  }

  void endResize() {
    state = const ResizerState(
      resizingSide: null,
      wantedSize: Size.zero,
    );
  }

  Offset computeWindowOffset(Size oldSize, Size newSize) {
    Offset offset = (newSize - oldSize) as Offset;

    double dx = offset.dx;
    double dy = offset.dy;

    switch (state.resizingSide) {
      case ResizingSide.topLeft:
        return Offset(-dx, -dy);
      case ResizingSide.top:
      case ResizingSide.topRight:
        return Offset(0, -dy);
      case ResizingSide.left:
      case ResizingSide.bottomLeft:
        return Offset(-dx, 0);
      case ResizingSide.right:
      case ResizingSide.bottomRight:
      case ResizingSide.bottom:
      case null:
        return Offset.zero;
    }
  }

  Offset _computeResizeOffset(Offset delta) {
    double dx = delta.dx;
    double dy = delta.dy;

    switch (state.resizingSide) {
      case ResizingSide.topLeft:
        return Offset(-dx, -dy);
      case ResizingSide.top:
        return Offset(0, -dy);
      case ResizingSide.topRight:
        return Offset(dx, -dy);
      case ResizingSide.right:
        return Offset(dx, 0);
      case ResizingSide.bottomRight:
        return Offset(dx, dy);
      case ResizingSide.bottom:
        return Offset(0, dy);
      case ResizingSide.bottomLeft:
        return Offset(-dx, dy);
      case ResizingSide.left:
        return Offset(-dx, 0);
      case null:
        return Offset(dx, dy);
    }
  }
}

@freezed
class ResizerState with _$ResizerState {
  const factory ResizerState({
    required ResizingSide? resizingSide,
    required Size wantedSize,
  }) = _ResizerState;
}
