import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:visibility_detector/visibility_detector.dart';
import 'package:zenith/platform_api.dart';
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

  VisibilityDetectorController.instance.updateInterval = Duration.zero;

  SchedulerBinding.instance.addPostFrameCallback((_) {
    PlatformApi.startupComplete();
  });

  runApp(const ProviderScope(child: Zenith()));
}

const _notchHeight = 40.0; // physical pixels

class Zenith extends StatelessWidget {
  const Zenith({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Zenith',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      scrollBehavior: const MaterialScrollBehavior().copyWith(
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
      home: Builder(
        builder: (context) {
          return MediaQuery(
            data: MediaQuery.of(context).copyWith(
              padding: EdgeInsets.only(top: _notchHeight / MediaQuery.of(context).devicePixelRatio),
            ),
            child: const Scaffold(
              body: Desktop(),
            ),
          );
        },
      ),
    );
  }
}
