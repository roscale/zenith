import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/desktop/state/window_move_provider.dart';
import 'package:zenith/ui/desktop/window.dart';

class TitleBar extends ConsumerStatefulWidget {
  final int viewId;

  const TitleBar({super.key, required this.viewId});

  @override
  ConsumerState<ConsumerStatefulWidget> createState() {
    return _TitleBarState();
  }
}

class _TitleBarState extends ConsumerState<TitleBar> {
  var startPosition = Offset.zero;
  var delta = Offset.zero;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onPanDown: (DragDownDetails details) {
        Offset startPosition = ref.read(windowPositionStateProvider(widget.viewId));
        ref.read(windowMoveProvider(widget.viewId).notifier).startMove(startPosition);
      },
      onPanUpdate: (DragUpdateDetails details) {
        ref.read(windowMoveProvider(widget.viewId).notifier).move(details.delta);
      },
      onPanEnd: (_) {
        ref.read(windowMoveProvider(widget.viewId).notifier).endMove();
      },
      onPanCancel: () {
        ref.read(windowMoveProvider(widget.viewId).notifier).cancelMove();
      },
      child: SizedBox(
        height: 30,
        child: Material(
          color: Colors.white70,
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              SizedBox(
                width: 30,
                height: 30,
                child: IconButton(
                  padding: EdgeInsets.zero,
                  splashRadius: 22,
                  iconSize: 18,
                  icon: const Icon(Icons.close),
                  onPressed: () {
                    PlatformApi.closeView(widget.viewId);
                  },
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
