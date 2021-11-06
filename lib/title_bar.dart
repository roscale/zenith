import 'package:elinux_app/action_buttons.dart';
import 'package:elinux_app/window_state.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class TitleBar extends StatelessWidget {
  const TitleBar({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var windowState = context.watch<WindowState>();

    return GestureDetector(
      onPanUpdate: (DragUpdateDetails details) {
        windowState.rect = windowState.rect.shift(details.delta);
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
                children: [
                  Text(windowState.title, style: const TextStyle(color: Colors.white)),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
