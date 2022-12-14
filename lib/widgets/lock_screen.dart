import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:intl/intl.dart';
import 'package:zenith/state/lock_screen_state.dart';

class LockScreen extends ConsumerStatefulWidget {
  const LockScreen({super.key});

  @override
  ConsumerState<LockScreen> createState() => _LockScreenState();
}

class _LockScreenState extends ConsumerState<LockScreen> with SingleTickerProviderStateMixin {
  late AnimationController _showAuthenticationAnimationController;

  @override
  void initState() {
    super.initState();
    _showAuthenticationAnimationController = AnimationController(
      duration: const Duration(milliseconds: 200),
      vsync: this,
    );
  }

  @override
  void dispose() {
    _showAuthenticationAnimationController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Stack(
      fit: StackFit.expand,
      children: [
        Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
        Consumer(
          builder: (BuildContext context, WidgetRef ref, Widget? child) {
            double offset = ref.watch(lockScreenStateProvider.select((value) => value.offset));
            double slideDistance = ref.watch(lockScreenStateProvider.select((value) => value.slideDistance));
            return Transform.translate(
              offset: Offset(0, -offset),
              child: Opacity(
                opacity: (1.0 - offset / (slideDistance / 2)).clamp(0.0, 1.0),
                child: const Clock(),
              ),
            );
          },
        ),
        Consumer(
          builder: (BuildContext context, WidgetRef ref, Widget? child) {
            double offset = ref.watch(lockScreenStateProvider.select((value) => value.offset));
            double slideDistance = ref.watch(lockScreenStateProvider.select((value) => value.slideDistance));
            return Transform.translate(
              offset: Offset(0, slideDistance - offset),
              child: Opacity(
                opacity: (offset / (slideDistance / 2) - 1.0).clamp(0.0, 1.0),
                child: const PinAuthentication(),
              ),
            );
          },
        ),
        GestureDetector(
          onPanDown: (DragDownDetails details) {
            ref.read(lockScreenStateProvider.notifier).startDrag();
          },
          onPanUpdate: (DragUpdateDetails details) {
            ref.read(lockScreenStateProvider.notifier).drag(-details.delta.dy);
          },
          onPanEnd: (DragEndDetails details) {
            ref.read(lockScreenStateProvider.notifier).endDrag(details.velocity.pixelsPerSecond.dy);
          },
        )
      ],
    );
  }
}

class Clock extends StatefulWidget {
  const Clock({super.key});

  @override
  State<Clock> createState() => _ClockState();

  static const _shadow = Shadow(
    color: Colors.black45,
    blurRadius: 10,
  );
}

class _ClockState extends State<Clock> {
  final DateFormat _format = DateFormat.MMMMEEEEd();
  late Timer _timer;

  @override
  void initState() {
    super.initState();
    _timer = Timer.periodic(const Duration(seconds: 1), (_) => setState(() {}));
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final now = DateTime.now();

    return Stack(
      children: [
        Positioned(
          left: 50,
          top: 50,
          child: Text(
            _format.format(now),
            style: const TextStyle(
              color: Colors.white,
              fontSize: 20,
              shadows: [Clock._shadow],
            ),
          ),
        ),
        Align(
          alignment: Alignment.center,
          child: DefaultTextStyle(
            textAlign: TextAlign.center,
            style: const TextStyle(
              fontSize: 200,
              fontWeight: FontWeight.normal,
              height: 0.8,
              shadows: [Clock._shadow],
            ),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                Text("${now.hour}".padLeft(2, '0')),
                Text("${now.minute}".padLeft(2, '0')),
              ],
            ),
          ),
        ),
        const Positioned(
          bottom: 200,
          left: 0,
          right: 0,
          child: Icon(
            Icons.lock,
            color: Colors.white,
            size: 50,
            shadows: [Clock._shadow],
          ),
        ),
      ],
    );
  }
}

class PinAuthentication extends StatelessWidget {
  const PinAuthentication({super.key});

  @override
  Widget build(BuildContext context) {
    return Material(
      type: MaterialType.transparency,
      child: Stack(
        children: [
          Positioned(
            bottom: 100,
            left: 0,
            right: 0,
            child: Column(
              children: [
                Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    pinButton(Text("1")),
                    pinButton(Text("2")),
                    pinButton(Text("3")),
                  ],
                ),
                Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    pinButton(Text("4")),
                    pinButton(Text("5")),
                    pinButton(Text("6")),
                  ],
                ),
                Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    pinButton(Text("7")),
                    pinButton(Text("8")),
                    pinButton(Text("9")),
                  ],
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

Widget pinButton(Widget child) {
  return Padding(
    padding: const EdgeInsets.all(10.0),
    child: SizedBox.fromSize(
      size: Size(100, 100), // button width and height
      child: ClipOval(
        child: Material(
          color: Colors.white12, // button color
          child: InkWell(
            splashColor: Colors.white24, // splash color
            onTap: () {}, // button pressed
            child: Center(
              child: DefaultTextStyle(
                style: TextStyle(
                  color: Colors.white,
                  fontSize: 40,
                ),
                child: child,
              ),
            ),
          ),
        ),
      ),
    ),
  );
  return IconButton(
    onPressed: () {
      print("heh");
    },
    icon: child,
    iconSize: 60,
  );
}
