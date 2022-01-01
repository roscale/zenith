import 'package:flutter/rendering.dart';
import 'package:flutter/scheduler.dart';
import 'package:zenith/desktop_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:provider/provider.dart';
import 'window_stack.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  WidgetsBinding.instance!.addPersistentFrameCallback((timeStamp) {
    // FIXME: FlutterEngineMarkExternalTextureFrameAvailable does not trigger a VSync fast enough,
    // so Flutter will only VSync every second frame. Marking the texture after FlutterEngineOnVsync
    // only fixes the problem partially because Flutter will still skip frames every once in a while.
    // This forces Flutter to always schedule a new frame.
    WidgetsBinding.instance!.scheduleFrame();
  });
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Zenith',
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
      pointerPosition = Offset(
        event.position.dx.floorToDouble(),
        event.position.dy.floorToDouble(),
      );
    });
  }

  @override
  Widget build(BuildContext context) {
    var desktopState = context.watch<DesktopState>();

    return Stack(
      fit: StackFit.expand,
      children: [
        Container(color: Colors.grey),
        Listener(
          onPointerHover: pointerExit,
          child: Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
        ),
        ...desktopState.windows,
        ...desktopState.popups,
        Positioned(
          // This is a workaround for a Flutter bug.
          // This has to be here because otherwise the background image is not being redrawn.
          child: IgnorePointer(child: Container(color: Colors.white.withAlpha(1), width: 0.1, height: 0.1)),
          left: 0,
          top: 0,
        ),
      ],
    );
  }
}

void pointerExit(PointerEvent event) {
  DesktopState.platform.invokeMethod("pointer_exit");
}
