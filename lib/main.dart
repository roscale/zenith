import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/display_brightness_state.dart';
import 'package:zenith/widgets/desktop.dart';

void main() {
  // FIXME: FlutterEngineMarkExternalTextureFrameAvailable does not trigger a VSync fast enough,
  // so Flutter will only VSync every second frame. Marking a texture after FlutterEngineOnVsync
  // only fixes the problem partially because Flutter will still skip frames every once in a while.
  // This forces Flutter to always schedule a new frame.
  WidgetsFlutterBinding.ensureInitialized();
  WidgetsBinding.instance.addPersistentFrameCallback((_) {
    WidgetsBinding.instance.scheduleFrame();
  });

  SchedulerBinding.instance.addPostFrameCallback((_) {
    PlatformApi.startupComplete();
  });

  final container = ProviderContainer();
  bool on = true;
  double brightnessBackup = 0;

  HardwareKeyboard.instance.addHandler((KeyEvent keyEvent) {
    if (keyEvent is KeyDownEvent && keyEvent.logicalKey == LogicalKeyboardKey.powerOff) {
      if (on) {
        brightnessBackup = container.read(displayBrightnessStateProvider).brightness;
        container.read(displayBrightnessStateProvider.notifier).setBrightness(0);
      } else {
        container.read(displayBrightnessStateProvider.notifier).setBrightness(brightnessBackup);
      }
      on = !on;
      return true;
    }
    return false;
  });

  runApp(
    UncontrolledProviderScope(
      container: container,
      child: const Zenith(),
    ),
  );
}

const _notchHeight = 80.0; // physical pixels

class Zenith extends StatelessWidget {
  const Zenith({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    // FIXME:
    // We cannot use MaterialApp because it somehow captures the arrow keys and tab automatically,
    // therefore these keys don't get forwarded to the Wayland client.
    // Let's use a WidgetApp for now. We cannot anymore select UI elements via the keyboard, but we
    // don't care about that on a mobile phone.
    return WidgetsApp(
      color: Colors.blue,
      builder: (BuildContext context, Widget? child) {
        return ScrollConfiguration(
          behavior: const MaterialScrollBehavior().copyWith(
            // Enable scrolling by dragging the mouse cursor.
            dragDevices: {
              PointerDeviceKind.touch,
              PointerDeviceKind.mouse,
              PointerDeviceKind.stylus,
              PointerDeviceKind.invertedStylus,
              PointerDeviceKind.trackpad,
              PointerDeviceKind.unknown,
            },
          ),
          child: MediaQuery(
            data: MediaQuery.of(context).copyWith(
              padding: EdgeInsets.only(top: _notchHeight / MediaQuery.of(context).devicePixelRatio),
            ),
            child: const Scaffold(
              body: Desktop(),
            ),
          ),
        );
      },
    );
  }
}
