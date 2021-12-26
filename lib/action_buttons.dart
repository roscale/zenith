import 'package:zenith/desktop_state.dart';
import 'package:zenith/window.dart';
import 'package:zenith/window_state.dart';
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
            var windowState = context.read<WindowState>();
            DesktopState.platform.invokeMethod("close_window", windowState.viewId);
          },
          icon: const Icon(Icons.close, color: Colors.white),
          splashRadius: 25,
        ),
      ],
    );
  }
}
