import 'dart:io';
import 'dart:ui';

import 'package:flutter/material.dart';

class QuickSettings extends StatefulWidget {
  const QuickSettings({Key? key}) : super(key: key);

  @override
  State<QuickSettings> createState() => _QuickSettingsState();
}

class _QuickSettingsState extends State<QuickSettings> {
  final maxBrightnessFile = File("/sys/class/backlight/intel_backlight/max_brightness");
  final brightnessFile = File("/sys/class/backlight/intel_backlight/brightness");

  late int maxBrightness = int.parse(maxBrightnessFile.readAsStringSync());
  late double brightnessFraction = double.parse(brightnessFile.readAsStringSync()) / maxBrightness;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.all(8.0),
      child: ClipRect(
        child: BackdropFilter(
          filter: ImageFilter.blur(sigmaX: 50, sigmaY: 50),
          child: Card(
            // clipBehavior: Clip.hardEdge,
            color: Colors.white54,
            shadowColor: Colors.transparent,
            margin: const EdgeInsets.all(0),
            child: Padding(
              padding: const EdgeInsets.all(30.0),
              child: Column(
                children: [
                  Row(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      IconButton(
                        onPressed: () {},
                        icon: const Icon(Icons.wifi, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: const Icon(Icons.import_export, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: const Icon(Icons.bluetooth, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: const Icon(Icons.screen_lock_rotation, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: const Icon(Icons.battery_full, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: const Icon(Icons.airplanemode_active, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                    ],
                  ),
                  const SizedBox(height: 20),
                  Row(
                    children: [
                      const Icon(Icons.brightness_6),
                      Expanded(
                        child: Slider(
                          value: brightnessFraction,
                          onChanged: (value) {
                            setState(() {
                              brightnessFraction = value;
                              brightnessFile.writeAsString("${(brightnessFraction * maxBrightness).floor()}");
                            });
                          },
                        ),
                      ),
                      const Icon(Icons.brightness_7),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
