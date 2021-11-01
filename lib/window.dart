import 'package:elinux_app/desktop_state.dart';
import 'package:elinux_app/title_bar.dart';
import 'package:elinux_app/window_state.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'dart:math';

class Window extends StatefulWidget {
  final int initialWidth;
  final int initialHeight;
  final int textureId;

  const Window({
    Key? key,
    required this.textureId,
    required this.initialWidth,
    required this.initialHeight,
  }) : super(key: key);

  @override
  _WindowState createState() => _WindowState();
}

class _WindowState extends State<Window> {
  late WindowState windowState;

  @override
  void initState() {
    super.initState();
    windowState = WindowState(
        "Window",
        Rect.fromLTWH(
          100,
          100,
          widget.initialWidth.toDouble(),
          widget.initialHeight.toDouble(),
        ),
        widget.textureId);
  }

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider.value(
      value: windowState,
      child: Builder(builder: (context) {
        var windowState = context.watch<WindowState>();

        return Positioned(
          child: GestureDetector(
            onPanDown: (_) =>
                context.read<DesktopState>().activateWindow(widget),
            child: Material(
              elevation: 20,
              child: SizedBox(
                width: windowState.rect.width,
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    const TitleBar(),
                    if (windowState.textureId != 0)
                      SizedBox(
                        width: windowState.rect.width,
                        height: windowState.rect.height,
                        child: Texture(
                          filterQuality: FilterQuality.none,
                          textureId: windowState.textureId,
                        ),
                      ),
                    if (windowState.textureId == 0)
                      SizedBox(
                        width: windowState.rect.width,
                        height: windowState.rect.height,
                        child: Container(color: Colors.red),
                      ),
                  ],
                ),
              ),
            ),
          ),
          left: windowState.rect.left,
          top: windowState.rect.top,
          // width: windowState.rect.width,
          // height: windowState.rect.height,
        );
      }),
    );
  }
}
