import 'package:flutter/material.dart';
import 'package:zenith/display_brightness.dart';

class QuickSettings extends StatefulWidget {
  final VoidCallback? onChangeBrightnessStart;
  final VoidCallback? onChangeBrightnessEnd;

  const QuickSettings({
    Key? key,
    this.onChangeBrightnessStart,
    this.onChangeBrightnessEnd,
  }) : super(key: key);

  @override
  State<QuickSettings> createState() => _QuickSettingsState();
}

class _QuickSettingsState extends State<QuickSettings> {
  final Future<DisplayBrightnessController> displayBrightnessControllerFuture =
      DisplayBrightnessController.getDefault();

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.all(8.0),
      child: Card(
        elevation: 0,
        color: Color.lerp(Colors.white, Colors.black, 0.2),
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
              _buildBrightnessSlider(),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildBrightnessSlider() {
    return FutureBuilder(
      future: displayBrightnessControllerFuture,
      builder: (BuildContext context, AsyncSnapshot<DisplayBrightnessController> snapshot) {
        if (snapshot.hasError || !snapshot.hasData) {
          // Don't show the slider if the display doesn't support changing the brightness.
          return const SizedBox();
        }
        final controller = snapshot.data!;
        return Column(
          children: [
            const SizedBox(height: 20),
            Row(
              children: [
                const Icon(Icons.brightness_6),
                Expanded(
                  child: ValueListenableBuilder(
                    valueListenable: controller.brightness,
                    builder: (BuildContext context, double brightness, __) {
                      return Slider(
                        value: brightness,
                        onChanged: (double value) => controller.setBrightness(value),
                        onChangeStart: (_) {
                          if (widget.onChangeBrightnessStart != null) {
                            widget.onChangeBrightnessStart!();
                          }
                        },
                        onChangeEnd: (_) {
                          if (widget.onChangeBrightnessEnd != null) {
                            widget.onChangeBrightnessEnd!();
                          }
                        },
                      );
                    },
                  ),
                ),
                const Icon(Icons.brightness_7),
              ],
            ),
          ],
        );
      },
    );
  }
}
