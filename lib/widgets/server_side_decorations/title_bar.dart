import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/widgets/server_side_decorations/action_buttons.dart';

class TitleBar extends StatelessWidget {
  const TitleBar({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var windowState = context.watch<WindowState>();

    return GestureDetector(
      onPanUpdate: (DragUpdateDetails details) {
        // windowState.position += details.delta;
      },
      child: SizedBox(
        height: 40,
        child: Material(
          color: Colors.grey.shade800,
          child: Stack(
            alignment: AlignmentDirectional.centerStart,
            children: [
              const ActionButtons(),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: const [
                  Text("Window", style: TextStyle(color: Colors.white)),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
