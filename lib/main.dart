import 'package:elinux_app/desktop_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:provider/provider.dart';

import 'custom_stack.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: ChangeNotifierProvider(
        create: (_) => DesktopState(),
        child: const Desktop(),
      ),
    );
  }
}

class Desktop extends StatefulWidget {
  const Desktop({Key? key}) : super(key: key);

  @override
  _DesktopState createState() => _DesktopState();
}

class _DesktopState extends State<Desktop> {
  Offset pointerPosition = Offset.zero;

  void cursorMoved(PointerEvent event) {
    setState(() {
      pointerPosition = event.position;
    });
  }

  @override
  Widget build(BuildContext context) {
    var desktopState = context.watch<DesktopState>();

    return Container(
      color: Colors.grey,
      child: CustomStack(
        children: [
          ...desktopState.windows,
          const Positioned(
            left: 0,
            top: 0,
            child: IgnorePointer(child: CircularProgressIndicator()),
          ),
          Positioned(
            child: Container(color: Colors.black, width: 10, height: 10),
            left: pointerPosition.dx,
            top: pointerPosition.dy,
          ),
          Listener(
            behavior: HitTestBehavior.translucent,
            onPointerHover: cursorMoved,
            onPointerMove: cursorMoved,
          ),
        ],
      ),
    );
  }
}
