import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/desktop_state.dart';

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
    return ChangeNotifierProvider(
      create: (_) => DesktopState(),
      child: Builder(
        builder: (BuildContext context) {
          var desktopState = context.watch<DesktopState>();

          return Stack(
            fit: StackFit.expand,
            children: [
              Container(color: Colors.grey),
              Listener(
                onPointerHover: pointerExit,
                child: Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
              ),
              // Listener(onPointerHover: pointerExit, child: Positioned.fill(child: Container()),),
              ...desktopState.windows,
              ...desktopState.popups,
              Positioned(
                // FIXME: This is a workaround for a Flutter bug.
                // This has to be here because otherwise the background image is not being redrawn.
                child: IgnorePointer(child: Container(color: Colors.white.withAlpha(1), width: 0.1, height: 0.1)),
                left: 0,
                top: 0,
              ),
            ],
          );
        },
      ),
    );
  }
}

void pointerExit(PointerEvent event) {
  DesktopState.platform.invokeMethod("pointer_exit");
}
