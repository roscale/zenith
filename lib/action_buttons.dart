import 'package:elinux_app/desktop_state.dart';
import 'package:elinux_app/window.dart';
import 'package:elinux_app/window_state.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class ActionButtons extends StatelessWidget {
  const ActionButtons({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        IconButton(
          onPressed: () {
            var window = context.findAncestorWidgetOfExactType<Window>();
            if (window != null) {
              DesktopState.platform.invokeMethod("close_window", window.viewPtr);
            }
          },
          icon: const Icon(Icons.close, color: Colors.white),
          splashRadius: 25,
        ),
        // IconButton(
        //   onPressed: () {},
        //   icon: const Icon(Icons.crop_square, color: Colors.white),
        //   splashRadius: 25,
        // ),
        // IconButton(
        //   onPressed: () {},
        //   icon: const Icon(Icons.minimize, color: Colors.white),
        //   splashRadius: 25,
        // ),
      ],
    );
  }
}
