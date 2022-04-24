import 'dart:ui';

import 'package:flutter/material.dart';

class QuickSettings extends StatefulWidget {
  const QuickSettings({Key? key}) : super(key: key);

  @override
  State<QuickSettings> createState() => _QuickSettingsState();
}

class _QuickSettingsState extends State<QuickSettings> {
  var brightness = 0.4;

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
                        icon: Icon(Icons.wifi, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: Icon(Icons.import_export, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: Icon(Icons.bluetooth, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: Icon(Icons.screen_lock_rotation, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: Icon(Icons.battery_full, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                      IconButton(
                        onPressed: () {},
                        icon: Icon(Icons.airplanemode_active, size: 30),
                        padding: EdgeInsets.zero,
                      ),
                    ],
                  ),
                  SizedBox(height: 20),
                  Row(
                    children: [
                      Icon(Icons.brightness_6),
                      Expanded(
                        child: Slider(
                          value: brightness,
                          onChanged: (value) {
                            setState(() {
                              brightness = value;
                            });
                          },
                        ),
                      ),
                      Icon(Icons.brightness_7),
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
