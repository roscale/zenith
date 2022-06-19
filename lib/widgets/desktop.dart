import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/state/desktop_state.dart';
import 'package:zenith/system_ui/task_switcher/task_switcher.dart';
import 'package:zenith/system_ui/top.dart';

class Desktop extends StatelessWidget {
  const Desktop({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Provider(
      create: (_) => DesktopState(),
      dispose: (_, DesktopState value) => value.dispose(),
      lazy: false,
      child: Stack(
        fit: StackFit.expand,
        children: [
          Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
          const Padding(
            padding: EdgeInsets.only(top: 40.0),
            child: TaskSwitcher(
              spacing: 20,
            ),
          ),
          const Top(),
        ],
      ),
    );
  }
}
