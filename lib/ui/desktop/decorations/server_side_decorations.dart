import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/zenith_xdg_surface_state.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
import 'package:zenith/ui/desktop/activate_and_raise.dart';
import 'package:zenith/ui/desktop/decorations/title_bar.dart';
import 'package:zenith/ui/desktop/state/window_resize_provider.dart';
import 'package:zenith/util/rect_overflow_box.dart';

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
    return ActivateAndRaise(
      viewId: viewId,
      child: Stack(
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
                    UnconstrainedBox(
                      child: ClipRect(
                        child: Consumer(
                          builder: (BuildContext context, WidgetRef ref, Widget? child) {
                            Rect visibleBounds =
                                ref.watch(zenithXdgSurfaceStateProvider(viewId).select((value) => value.visibleBounds));
                            return RectOverflowBox(
                              rect: visibleBounds,
                              child: child!,
                            );
                          },
                          child: child,
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
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
                child: ResizeHandle(viewId: viewId, resizeEdge: ResizeEdge.topLeft),
              ),
              Expanded(
                child: ResizeHandle(viewId: viewId, resizeEdge: ResizeEdge.top),
              ),
              Container(
                constraints: BoxConstraints(maxWidth: cornerWidth),
                child: ResizeHandle(viewId: viewId, resizeEdge: ResizeEdge.topRight),
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
                  resizeEdge: ResizeEdge.bottomLeft,
                ),
              ),
              Expanded(
                child: ResizeHandle(
                  viewId: viewId,
                  resizeEdge: ResizeEdge.bottom,
                ),
              ),
              Container(
                constraints: BoxConstraints(maxWidth: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizeEdge: ResizeEdge.bottomRight,
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
                  resizeEdge: ResizeEdge.topLeft,
                ),
              ),
              Expanded(
                child: ResizeHandle(
                  viewId: viewId,
                  resizeEdge: ResizeEdge.left,
                ),
              ),
              Container(
                constraints: BoxConstraints(maxHeight: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizeEdge: ResizeEdge.bottomLeft,
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
                  resizeEdge: ResizeEdge.topRight,
                ),
              ),
              Expanded(
                child: ResizeHandle(
                  viewId: viewId,
                  resizeEdge: ResizeEdge.right,
                ),
              ),
              Container(
                constraints: BoxConstraints(maxHeight: cornerWidth),
                child: ResizeHandle(
                  viewId: viewId,
                  resizeEdge: ResizeEdge.bottomRight,
                ),
              ),
            ],
          ),
        ),
      ),
    ];
  }
}

class ResizeHandle extends ConsumerWidget {
  final int viewId;
  final ResizeEdge resizeEdge;

  const ResizeHandle({
    super.key,
    required this.viewId,
    required this.resizeEdge,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return MouseRegion(
      cursor: _getMouseCursor(),
      child: GestureDetector(
        onPanDown: (_) {
          Size size = ref.read(zenithXdgSurfaceStateProvider(viewId)).visibleBounds.size;
          ref.read(windowResizeProvider(viewId).notifier).startResize(resizeEdge, size);
        },
        onPanUpdate: (DragUpdateDetails details) {
          ref.read(windowResizeProvider(viewId).notifier).resize(details.delta);
        },
        onPanEnd: (_) {
          ref.read(windowResizeProvider(viewId).notifier).endResize();
        },
        onPanCancel: () {
          ref.read(windowResizeProvider(viewId).notifier).endResize();
        },
      ),
    );
  }

  MouseCursor _getMouseCursor() {
    switch (resizeEdge) {
      case ResizeEdge.topLeft:
        return SystemMouseCursors.resizeUpLeft;
      case ResizeEdge.top:
        return SystemMouseCursors.resizeUp;
      case ResizeEdge.topRight:
        return SystemMouseCursors.resizeUpRight;
      case ResizeEdge.right:
        return SystemMouseCursors.resizeRight;
      case ResizeEdge.bottomRight:
        return SystemMouseCursors.resizeDownRight;
      case ResizeEdge.bottom:
        return SystemMouseCursors.resizeDown;
      case ResizeEdge.bottomLeft:
        return SystemMouseCursors.resizeDownLeft;
      case ResizeEdge.left:
        return SystemMouseCursors.resizeLeft;
    }
  }
}
