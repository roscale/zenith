import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/ui/common/app_icon.dart';
import 'package:zenith/ui/common/state/zenith_xdg_toplevel_state.dart';
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
        child: BackdropFilter(
          filter: ImageFilter.blur(
            sigmaX: 3.0,
            sigmaY: 3.0,
          ),
          child: Material(
            color: Colors.white54,
            child: Stack(
              children: [
                Consumer(
                  builder: (BuildContext context, WidgetRef ref, Widget? child) {
                    String title = ref.watch(zenithXdgToplevelStateProvider(widget.viewId).select((v) => v.title));

                    return Positioned.fill(
                      child: Align(
                        alignment: Alignment.center,
                        child: Text(
                          title,
                          overflow: TextOverflow.ellipsis,
                          style: const TextStyle(
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ),
                    );
                  },
                ),
                Row(
                  crossAxisAlignment: CrossAxisAlignment.center,
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
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
                    Consumer(
                      builder: (BuildContext context, WidgetRef ref, Widget? child) {
                        String appId = ref.watch(zenithXdgToplevelStateProvider(widget.viewId).select((v) => v.appId));
                        return SizedBox(
                          height: 30,
                          width: 30,
                          child: Padding(
                            padding: const EdgeInsets.all(2.0),
                            child: AppIconById(id: appId),
                          ),
                        );
                      },
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
