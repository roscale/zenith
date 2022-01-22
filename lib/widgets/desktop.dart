import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/desktop_state.dart';

class Desktop extends StatelessWidget {
  const Desktop({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => DesktopState(),
      child: Builder(
        builder: (BuildContext context) {
          var desktopState = context.watch<DesktopState>();

          return Listener(
            onPointerHover: (event) => pointerMoved(context, event),
            onPointerMove: (event) => pointerMoved(context, event),
            onPointerUp: (event) => pointerUp(context, event),
            child: Stack(
              fit: StackFit.expand,
              children: [
                Container(color: Colors.grey),
                Listener(
                  onPointerHover: pointerExitSurfaces,
                  child: Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
                ),
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
            ),
          );
        },
      ),
    );
  }

  void pointerMoved(BuildContext context, PointerEvent event) {
    context.read<DesktopState>().pointerPosition = Offset(
      event.position.dx.floorToDouble(),
      event.position.dy.floorToDouble(),
    );
  }

  void pointerExitSurfaces(PointerEvent event) {
    // When the pointer is hovering the background, it exited all surfaces.
    DesktopState.platform.invokeMethod("pointer_exit");
  }

  void pointerUp(BuildContext context, PointerUpEvent event) {
    context.read<DesktopState>().pointerUpStream.sink.add(null);
  }
}
