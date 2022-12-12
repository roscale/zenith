import 'dart:io';
import 'dart:math';

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'display_brightness_state.freezed.dart';

final displayBrightnessStateProvider =
    StateNotifierProvider<DisplayBrightnessStateNotifier, DisplayBrightnessState>((ref) {
  return DisplayBrightnessStateNotifier();
});

@freezed
class DisplayBrightnessState with _$DisplayBrightnessState {
  const factory DisplayBrightnessState({
    required bool available,
    required File brightnessFile,
    required int maxBrightness,
    required double brightness,
  }) = _DisplayBrightnessState;
}

const _backlightsDirectory = "/sys/class/backlight";

class DisplayBrightnessStateNotifier extends StateNotifier<DisplayBrightnessState> {
  DisplayBrightnessStateNotifier()
      : super(
          DisplayBrightnessState(
            available: false,
            brightnessFile: File(""),
            maxBrightness: 0,
            brightness: 0.0,
          ),
        ) {
    _init();
  }

  Future<void> _init() async {
    try {
      DisplayBrightnessState state = await _getDefault();
      state.brightnessFile.watch(events: FileSystemEvent.modify).forEach((_) async {
        String brightnessString = await state.brightnessFile.readAsString();
        double measuredBrightness = int.parse(brightnessString) / state.maxBrightness;
        measuredBrightness = measuredBrightness.clamp(0.0, 1.0);
        this.state = state.copyWith(brightness: _getPerceivedBrightness(measuredBrightness));
      });
      this.state = state;
    } catch (e) {
      // Retry.
      await Future.delayed(const Duration(seconds: 1));
      _init();
    }
  }

  double get brightness => state.brightness;

  Future<void> setBrightness(double value) async {
    int measuredBrightness = (_getMeasuredBrightness(value) * state.maxBrightness).round();
    await state.brightnessFile.writeAsString("$measuredBrightness", flush: true);
  }
}

Future<DisplayBrightnessState> _getDefault() async {
  FileSystemEntity backlight = await Directory(_backlightsDirectory).list().first;

  File brightnessFile = File("${backlight.absolute.path}/brightness");
  File maxBrightnessFile = File("${backlight.absolute.path}/max_brightness");

  int maxBrightness = int.parse(await maxBrightnessFile.readAsString());
  double measuredBrightness = int.parse(await brightnessFile.readAsString()) / maxBrightness;

  return DisplayBrightnessState(
    available: true,
    brightnessFile: brightnessFile,
    maxBrightness: maxBrightness,
    brightness: _getPerceivedBrightness(measuredBrightness),
  );
}

double _getPerceivedBrightness(double measuredBrightness) => sqrt(measuredBrightness);

double _getMeasuredBrightness(double perceivedBrightness) => perceivedBrightness * perceivedBrightness;
