import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:visibility_detector/visibility_detector.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/key_tracker.dart';
import 'package:zenith/state/lock_screen_state.dart';
import 'package:zenith/state/power_menu_state.dart';
import 'package:zenith/state/root_overlay.dart';
import 'package:zenith/state/screen_state.dart';
import 'package:zenith/widgets/desktop.dart';

void main() {
  // debugRepaintRainbowEnabled = true;
  // debugPrintGestureArenaDiagnostics = true;
  WidgetsFlutterBinding.ensureInitialized();

  SchedulerBinding.instance.addPostFrameCallback((_) {
    PlatformApi.startupComplete();
  });

  final container = ProviderContainer();
  PlatformApi.init(container);

  _registerLockScreenKeyboardHandler(container);
  _registerPowerButtonHandler(container);

  VisibilityDetectorController.instance.updateInterval = Duration.zero;

  runApp(
    UncontrolledProviderScope(
      container: container,
      child: Builder(
        builder: (context) {
          return Consumer(
            builder: (BuildContext context, WidgetRef ref, Widget? child) {
              bool screenOn = ref.watch(screenStateProvider.select((v) => v.on));
              final screenStateNotifier = ref.read(screenStateProvider.notifier);

              return GestureDetector(
                onDoubleTap: !screenOn ? () => screenStateNotifier.turnOn() : null,
                child: AbsorbPointer(
                  absorbing: !screenOn,
                  child: child,
                ),
              );
            },
            child: Zenith(),
          );
        },
      ),
    ),
  );
}

const _notchHeight = 80.0; // physical pixels

class Zenith extends ConsumerWidget {
  final GlobalKey<OverlayState> overlayKey = GlobalKey();

  Zenith({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    // FIXME:
    // We cannot use MaterialApp because it somehow captures the arrow keys and tab automatically,
    // therefore these keys don't get forwarded to the Wayland client.
    // Let's use a WidgetApp for now. We cannot anymore select UI elements via the keyboard, but we
    // don't care about that on a mobile phone.
    return RotatedBox(
      quarterTurns: ref.watch(screenStateProvider.select((v) => v.rotation)),
      child: LayoutBuilder(
        builder: (BuildContext context, BoxConstraints constraints) {
          Future.microtask(() => ref.read(screenStateProvider.notifier).setRotatedSize(constraints.biggest));

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
                    padding: EdgeInsets.only(
                      top: _notchHeight / MediaQuery.of(context).devicePixelRatio,
                    ),
                  ),
                  // https://docs.flutter.dev/release/breaking-changes/text-field-material-localizations
                  child: Localizations(
                    locale: const Locale('en', 'US'),
                    delegates: const <LocalizationsDelegate<dynamic>>[
                      DefaultWidgetsLocalizations.delegate,
                      DefaultMaterialLocalizations.delegate,
                    ],
                    child: Scaffold(
                      body: Consumer(
                        builder: (BuildContext context, WidgetRef ref, Widget? child) {
                          return Overlay(
                            key: ref.watch(rootOverlayKeyProvider),
                            initialEntries: [
                              OverlayEntry(builder: (_) => const Desktop()),
                              // ref.read(lockScreenStateProvider).overlayEntry, // Start with the session locked.
                            ],
                          );
                        },
                      ),
                    ),
                  ),
                ),
              );
            },
          );
        },
      ),
    );
  }
}

void _registerLockScreenKeyboardHandler(ProviderContainer container) {
  HardwareKeyboard.instance.addHandler((KeyEvent keyEvent) {
    if (container.read(lockScreenStateProvider).locked) {
      // We don't want to send keyboard events to Wayland clients when the screen
      // is locked. Capture all keyboard events.
      return true;
    }
    return false;
  });
}

void _registerPowerButtonHandler(ProviderContainer container) {
  const KeyboardKey powerKey = LogicalKeyboardKey.powerOff;

  HardwareKeyboard.instance.addHandler((KeyEvent keyEvent) {
    if (keyEvent.logicalKey == powerKey) {
      if (keyEvent is KeyDownEvent) {
        container.read(keyTrackerProvider(keyEvent.logicalKey).notifier).down();
      } else if (keyEvent is KeyUpEvent) {
        container.read(keyTrackerProvider(keyEvent.logicalKey).notifier).up();
      }
      return true;
    }
    return false;
  });

  bool turnedOn = false;

  container.listen(keyTrackerProvider(powerKey).select((v) => v.down), (_, __) {
    final screenState = container.read(screenStateProvider);
    final screenStateNotifier = container.read(screenStateProvider.notifier);
    if (!screenState.on) {
      turnedOn = true;
      screenStateNotifier.turnOn();
      container.read(powerMenuStateProvider.notifier).removeOverlay();
    } else {
      turnedOn = false;
    }
  });

  container.listen(keyTrackerProvider(powerKey).select((v) => v.shortPress), (_, __) {
    final screenState = container.read(screenStateProvider);
    final screenStateNotifier = container.read(screenStateProvider.notifier);
    if (screenState.on && !turnedOn) {
      screenStateNotifier.lockAndTurnOff();
      container.read(powerMenuStateProvider.notifier).removeOverlay();
    }
  });

  container.listen(keyTrackerProvider(powerKey).select((v) => v.longPress), (_, __) {
    final state = container.read(powerMenuStateProvider);
    final notifier = container.read(powerMenuStateProvider.notifier);

    if (!state.shown) {
      notifier.show();
    } else {
      notifier.hide();
    }
  });
}
