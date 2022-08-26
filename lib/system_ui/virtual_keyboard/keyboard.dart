import 'dart:math' as math;

import 'package:flutter/material.dart';
import 'package:zenith/system_ui/virtual_keyboard/key.dart';
import 'package:zenith/system_ui/virtual_keyboard/key_codes.dart';

class VirtualKeyboard extends StatelessWidget {
  final VoidCallback onDismiss;
  final void Function(String) onCharacter;
  final void Function(KeyCode) onKeyCode;

  const VirtualKeyboard({
    Key? key,
    required this.onDismiss,
    required this.onCharacter,
    required this.onKeyCode,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: double.infinity,
      child: LayoutBuilder(
        builder: (BuildContext context, BoxConstraints constraints) {
          double keyWidth = constraints.maxWidth / 10;

          final firstRow = <Widget>[];
          for (int i = 1; i < 10; i++) {
            firstRow.add(VirtualKeyboardKey(
              width: keyWidth,
              onTap: () => onCharacter("$i"),
              child: Text("$i"),
            ));
          }
          firstRow.add(VirtualKeyboardKey(
            width: keyWidth,
            onTap: () => onCharacter("0"),
            child: const Text("0"),
          ));

          final secondRow = <Widget>[];
          for (String c in ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p"]) {
            secondRow.add(VirtualKeyboardKey(
              width: keyWidth,
              onTap: () => onCharacter(c),
              child: Text(c),
            ));
          }

          final thirdRow = <Widget>[];
          for (String c in ["a", "s", "d", "f", "g", "h", "j", "k", "l"]) {
            thirdRow.add(VirtualKeyboardKey(
              width: keyWidth,
              onTap: () => onCharacter(c),
              child: Text(c),
            ));
          }

          final forthRow = <Widget>[];
          forthRow.add(VirtualKeyboardKey(
            width: keyWidth * 1.5,
            popUpOnPress: false,
            child: Transform.rotate(
              angle: -90 * math.pi / 180,
              child: const Icon(Icons.forward_outlined),
            ),
          ));
          for (String c in ["z", "x", "c", "v", "b", "n", "m"]) {
            forthRow.add(VirtualKeyboardKey(
              width: keyWidth,
              onTap: () => onCharacter(c),
              child: Text(c),
            ));
          }
          forthRow.add(VirtualKeyboardKey(
            width: keyWidth * 1.5,
            popUpOnPress: false,
            repeatOnLongPress: true,
            onTap: () => onKeyCode(KeyCode.backspace),
            child: const Icon(Icons.backspace_outlined),
          ));

          final fifthRow = <Widget>[];
          fifthRow.addAll([
            VirtualKeyboardKey(
              width: keyWidth * 1.5,
              popUpOnPress: false,
              child: const Text(
                "?123",
                style: TextStyle(fontSize: 17),
              ),
            ),
            VirtualKeyboardKey(
              width: keyWidth,
              onTap: () => onCharacter(","),
              child: const Text(","),
            ),
            VirtualKeyboardKey(
              width: keyWidth,
              popUpOnPress: false,
              child: const Icon(Icons.language),
            ),
            Expanded(
              child: VirtualKeyboardKey(
                width: double.infinity,
                popUpOnPress: false,
                onTap: () => onCharacter(" "),
                child: const Text(" "),
              ),
            ),
            VirtualKeyboardKey(
              width: keyWidth,
              onTap: () => onCharacter("."),
              child: const Text("."),
            ),
            VirtualKeyboardKey(
              width: keyWidth * 1.5,
              popUpOnPress: false,
              onTap: () => onKeyCode(KeyCode.enter),
              child: const Icon(Icons.keyboard_return),
            ),
          ]);

          return Material(
            color: Color.lerp(Colors.black, Colors.white, 0.9),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                Row(children: firstRow),
                Row(children: secondRow),
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: thirdRow,
                ),
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: forthRow,
                ),
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: fifthRow,
                ),
                Row(
                  mainAxisAlignment: MainAxisAlignment.end,
                  children: [
                    IconButton(
                      icon: const Icon(Icons.keyboard_arrow_down, size: 30),
                      padding: EdgeInsets.zero,
                      onPressed: onDismiss,
                    ),
                  ],
                ),
                const SizedBox(height: 20),
              ],
            ),
          );
        },
      ),
    );
  }
}
