import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/desktop_state.dart';
import 'package:zenith/system_ui/task_switcher.dart';
import 'package:zenith/system_ui/top.dart';

final taskSwitcherKey = GlobalKey<TaskSwitcherState>();

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
                Listener(
                  onPointerHover: pointerExitSurfaces,
                  child: Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
                ),
                Padding(
                  padding: const EdgeInsets.only(top: 40.0),
                  child: TaskSwitcher(
                    key: taskSwitcherKey,
                    spacing: 20,
                  ),
                ),
                const Top(),
                // const Bottom(),
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

  void pointerUp(BuildContext context, PointerUpEvent event) {
    context.read<DesktopState>().pointerUpStream.sink.add(event);
  }
}

void pointerExitSurfaces(PointerEvent event) {
  // When the pointer is hovering the background, it exited all surfaces.
  DesktopState.platform.invokeMethod("pointer_exit");
}
