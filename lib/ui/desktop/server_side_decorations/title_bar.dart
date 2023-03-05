import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
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
        startPosition = ref.read(windowPositionStateProvider(widget.viewId));
        delta = Offset.zero;
      },
      onPanUpdate: (DragUpdateDetails details) {
        delta += details.delta;
        var offset = startPosition + delta;
        offset = Offset(offset.dx.roundToDouble(), offset.dy.roundToDouble());
        ref.read(windowPositionStateProvider(widget.viewId).notifier).state = offset;
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
