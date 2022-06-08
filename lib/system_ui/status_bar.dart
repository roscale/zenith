import 'package:flutter/material.dart';
import 'package:zenith/widgets/digital_clock.dart';

class StatusBar extends StatelessWidget {
  final GestureDragStartCallback onVerticalDragStart;
  final GestureDragUpdateCallback onVerticalDragUpdate;
  final GestureDragEndCallback onVerticalDragEnd;

  const StatusBar({
    Key? key,
    required this.onVerticalDragStart,
    required this.onVerticalDragUpdate,
    required this.onVerticalDragEnd,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onVerticalDragStart: onVerticalDragStart,
      onVerticalDragUpdate: onVerticalDragUpdate,
      onVerticalDragEnd: onVerticalDragEnd,
      child: Container(
        height: 40,
        color: Colors.black38,
        child: Row(
          children: const [
            SizedBox(width: 20),
            DigitalClock(),
            Spacer(),
            Icon(Icons.wifi, color: Colors.white),
            Icon(Icons.signal_cellular_4_bar, color: Colors.white),
            SizedBox(width: 10),
            Text(
              "98%",
              style: TextStyle(
                fontWeight: FontWeight.bold,
                color: Colors.white,
                fontSize: 18,
              ),
            ),
            Icon(Icons.battery_charging_full, color: Colors.white),
            SizedBox(width: 20),
          ],
        ),
      ),
    );
  }
}
