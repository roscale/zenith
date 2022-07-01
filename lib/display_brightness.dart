import 'dart:io';
import 'dart:math';

import 'package:flutter/foundation.dart';

const backlightsDirectory = "/sys/class/backlight";

/// Adjusts the brightness of a display.
/// It exposes the perceived brightness instead of the raw brightness because the eyes don't perceive
/// light intensity in a linear fashion.
/// Users of this class should not care about this because all conversions are done automatically.
/// More info at https://www.lutron.com/TechnicalDocumentLibrary/Measured_vs_Perceived.pdf
class DisplayBrightnessController {
  final File _brightnessFile;
  final int _maxBrightness;

  // Stores the perceived brightness fraction in the interval [0, 1].
  late ValueNotifier<double> _brightness;

  ValueListenable<double> get brightness => _brightness;

  Future<void> setBrightness(double brightness) async {
    int measuredBrightness = (getMeasuredBrightness(brightness) * _maxBrightness).round();
    await _brightnessFile.writeAsString("$measuredBrightness");
    _brightness.value = brightness;
  }

  DisplayBrightnessController._create(this._brightnessFile, this._maxBrightness);

  static Future<DisplayBrightnessController> getDefault() async {
    FileSystemEntity backlight = await Directory(backlightsDirectory).list().first;

    File brightnessFile = File("${backlight.absolute.path}${Platform.pathSeparator}brightness");
    File maxBrightnessFile = File("${backlight.absolute.path}${Platform.pathSeparator}max_brightness");
    int maxBrightness = int.parse(await maxBrightnessFile.readAsString());

    final instance = DisplayBrightnessController._create(brightnessFile, maxBrightness);
    double measuredBrightness = int.parse(await brightnessFile.readAsString()) / maxBrightness;
    instance._brightness = ValueNotifier(getPerceivedBrightness(measuredBrightness));

    brightnessFile.watch(events: FileSystemEvent.modify).forEach((_) async {
      double measuredBrightness = int.parse(await brightnessFile.readAsString()) / maxBrightness;
      instance._brightness.value = getPerceivedBrightness(measuredBrightness);
    });

    return instance;
  }
}

double getPerceivedBrightness(double measuredBrightness) => sqrt(measuredBrightness);

double getMeasuredBrightness(double perceivedBrightness) => perceivedBrightness * perceivedBrightness;
