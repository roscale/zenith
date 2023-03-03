import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/util/state/display_brightness_state.dart';
import 'package:zenith/util/state/screen_state.dart';

class QuickSettings extends ConsumerStatefulWidget {
  final VoidCallback? onChangeBrightnessStart;
  final VoidCallback? onChangeBrightnessEnd;

  const QuickSettings({
    Key? key,
    this.onChangeBrightnessStart,
    this.onChangeBrightnessEnd,
  }) : super(key: key);

  @override
  ConsumerState<QuickSettings> createState() => _QuickSettingsState();
}

class _QuickSettingsState extends ConsumerState<QuickSettings> {
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
                    onPressed: () => ref.read(screenStateProvider.notifier).rotateClockwise(),
                    icon: Transform.scale(
                      scaleX: -1,
                      child: const Icon(Icons.screen_rotation, size: 30),
                    ),
                    padding: EdgeInsets.zero,
                  ),
                  IconButton(
                    onPressed: () => ref.read(screenStateProvider.notifier).rotateCounterclockwise(),
                    icon: const Icon(Icons.screen_rotation, size: 30),
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
    return Consumer(
      builder: (BuildContext context, WidgetRef ref, Widget? child) {
        bool available = ref.watch(displayBrightnessStateProvider.select((v) => v.available));
        if (!available) {
          return const SizedBox();
        }
        double brightness = ref.watch(displayBrightnessStateProvider.select((v) => v.brightness));

        return Column(
          children: [
            const SizedBox(height: 20),
            Row(
              children: [
                const Icon(Icons.brightness_6),
                Expanded(
                  child: Slider(
                    value: brightness,
                    onChanged: (double value) => ref.read(displayBrightnessStateProvider.notifier).setBrightness(value),
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
