import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/ui/desktop/server_side_decorations/title_bar.dart';
import 'package:zenith/ui/desktop/state/resizing_state_notifier_provider.dart';

class ServerSideDecorations extends ConsumerWidget {
  final int viewId;
  final Widget child;
  final double borderWidth;
  final double cornerWidth;

  const ServerSideDecorations({
    super.key,
    required this.viewId,
    required this.child,
    this.borderWidth = 10,
    this.cornerWidth = 30,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Stack(
      children: [
        ..._buildResizeHandles(),
        Padding(
          padding: EdgeInsets.all(borderWidth),
          child: ClipRRect(
            borderRadius: BorderRadius.circular(5),
            child: IntrinsicWidth(
              child: Column(
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  TitleBar(
                    viewId: viewId,
                  ),
                  ClipRect(
                    child: UnconstrainedBox(
                      child: child,
                    ),
                  ),
                ],
              ),
            ),
          ),
        ),
      ],
    );
  }

  List<Widget> _buildResizeHandles() {
    return [
      Positioned(
        top: 0,
        left: 0,
        right: 0,
        child: SizedBox(
          height: borderWidth,
          child: Row(
            children: [
              Container(
                constraints: BoxConstraints(maxWidth: cornerWidth),
                child: ResizeHandle(viewId: viewId, resizingSide: ResizingSide.topLeft),
              ),
              Expanded(
                child: ResizeHandle(viewId: viewId, resizingSide: ResizingSide.top),
              ),
              Container(
                constraints: BoxConstraints(maxWidth: cornerWidth),
                child: ResizeHandle(viewId: viewId, resizingSide: ResizingSide.topRight),
              ),
            ],
          ),
        ),
      ),
      Positioned(
        bottom: 0,
        left: 0,
        right: 0,
        child: SizedBox(
          height: borderWidth,
          child: Row(
            children: [
              Container(
                constraints: BoxConstraints(maxWidth: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.bottomLeft,
                ),
              ),
              Expanded(
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.bottom,
                ),
              ),
              Container(
                constraints: BoxConstraints(maxWidth: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.bottomRight,
                ),
              ),
            ],
          ),
        ),
      ),
      Positioned(
        left: 0,
        top: 0,
        bottom: 0,
        child: SizedBox(
          width: borderWidth,
          child: Column(
            children: [
              Container(
                constraints: BoxConstraints(maxHeight: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.topLeft,
                ),
              ),
              Expanded(
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.left,
                ),
              ),
              Container(
                constraints: BoxConstraints(maxHeight: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.bottomLeft,
                ),
              ),
            ],
          ),
        ),
      ),
      Positioned(
        right: 0,
        top: 0,
        bottom: 0,
        child: SizedBox(
          width: borderWidth,
          child: Column(
            children: [
              Container(
                constraints: BoxConstraints(maxHeight: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.topRight,
                ),
              ),
              Expanded(
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.right,
                ),
              ),
              Container(
                constraints: BoxConstraints(maxHeight: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizingSide: ResizingSide.bottomRight,
                ),
              ),
            ],
          ),
        ),
      ),
    ];
  }
}

enum ResizingSide {
  topLeft,
  top,
  topRight,
  right,
  bottomRight,
  bottom,
  bottomLeft,
  left,
}

class ResizeHandle extends ConsumerWidget {
  final int viewId;
  final ResizingSide resizingSide;

  const ResizeHandle({
    super.key,
    required this.viewId,
    required this.resizingSide,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return MouseRegion(
      cursor: _getMouseCursor(),
      child: GestureDetector(
        onPanDown: (_) {
          Size size = ref.read(zenithXdgSurfaceStateProvider(viewId)).visibleBounds.size;
          ref.read(resizingStateNotifierProvider(viewId).notifier).startResize(resizingSide, size);
        },
        onPanUpdate: (DragUpdateDetails details) {
          ref.read(resizingStateNotifierProvider(viewId).notifier).resize(details.delta);
        },
        onPanEnd: (_) {
          ref.read(resizingStateNotifierProvider(viewId).notifier).endResize();
        },
        onPanCancel: () {
          ref.read(resizingStateNotifierProvider(viewId).notifier).endResize();
        },
      ),
    );
  }

  MouseCursor _getMouseCursor() {
    switch (resizingSide) {
      case ResizingSide.topLeft:
        return SystemMouseCursors.resizeUpLeft;
      case ResizingSide.top:
        return SystemMouseCursors.resizeUp;
      case ResizingSide.topRight:
        return SystemMouseCursors.resizeUpRight;
      case ResizingSide.right:
        return SystemMouseCursors.resizeRight;
      case ResizingSide.bottomRight:
        return SystemMouseCursors.resizeDownRight;
      case ResizingSide.bottom:
        return SystemMouseCursors.resizeDown;
      case ResizingSide.bottomLeft:
        return SystemMouseCursors.resizeDownLeft;
      case ResizingSide.left:
        return SystemMouseCursors.resizeLeft;
    }
  }
}
