import 'package:flutter/material.dart';

class VirtualKeyboardKey extends StatelessWidget {
  final double width;
  final Widget child;
  final VoidCallback? onTap;

  const VirtualKeyboardKey({
    Key? key,
    required this.width,
    required this.child,
    this.onTap,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: width,
      height: 50,
      child: Card(
        elevation: 2,
        margin: const EdgeInsets.all(3),
        child: InkWell(
          onTap: onTap,
          child: Center(
            child: DefaultTextStyle(
              style: const TextStyle(color: Colors.black, fontSize: 20),
              child: child,
            ),
          ),
        ),
      ),
    );
  }
}
