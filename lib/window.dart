import 'package:elinux_app/desktop_state.dart';
import 'package:elinux_app/title_bar.dart';
import 'package:elinux_app/window_state.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class Window extends StatefulWidget {
  const Window({Key? key}) : super(key: key);

  @override
  _WindowState createState() => _WindowState();
}

class _WindowState extends State<Window> {
  var windowState = WindowState("Window", const Rect.fromLTWH(100, 100, 500, 300));

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider.value(
      value: windowState,
      child: Builder(builder: (context) {
        var windowState = context.watch<WindowState>();

        return Positioned(
          child: GestureDetector(
            onPanDown: (_) => context.read<DesktopState>().activateWindow(widget),
            child: Card(
              clipBehavior: Clip.antiAlias,
              elevation: 20,
              child: Column(
                children: const [
                  TitleBar(),
                ],
              ),
            ),
          ),
          left: windowState.rect.left,
          top: windowState.rect.top,
          width: windowState.rect.width,
          height: windowState.rect.height,
        );
      }),
    );
  }
}
