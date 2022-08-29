import 'dart:math' as math;

import 'package:flutter/material.dart';
import 'package:zenith/system_ui/virtual_keyboard/key.dart';
import 'package:zenith/system_ui/virtual_keyboard/key_codes.dart';
import 'package:zenith/system_ui/virtual_keyboard/layouts.dart';
import 'package:zenith/util/multi_value_listenable_builder.dart';

class VirtualKeyboard extends StatefulWidget {
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
  State<VirtualKeyboard> createState() => VirtualKeyboardState();
}

class VirtualKeyboardState extends State<VirtualKeyboard> {
  var layout = ValueNotifier(layouts["en"]!);
  var layer = ValueNotifier(VirtualKeyboardLayer.first);
  var letterCase = ValueNotifier(Case.lowercase);

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: double.infinity,
      child: LayoutBuilder(
        builder: (BuildContext context, BoxConstraints constraints) {
          double keyWidth = constraints.maxWidth / 10;

          return Material(
            color: Color.lerp(Colors.black, Colors.white, 0.9),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                _buildKeys(keyWidth),
                Row(
                  mainAxisAlignment: MainAxisAlignment.end,
                  children: [
                    IconButton(
                      icon: const Icon(Icons.keyboard_arrow_down, size: 30),
                      padding: EdgeInsets.zero,
                      onPressed: widget.onDismiss,
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

  Widget _buildKeys(double keyWidth) {
    return ValueListenableBuilder3(
      first: layout,
      second: layer,
      third: letterCase,
      builder: (
        BuildContext context,
        VirtualKeyboardLayout layout,
        VirtualKeyboardLayer layer,
        Case letterCase,
        _,
      ) {
        List<List<Widget>> layerWidgets = [];

        switch (layer) {
          case VirtualKeyboardLayer.first:
            layerWidgets = buildFirstLayer(layout, keyWidth, letterCase);
            break;
          case VirtualKeyboardLayer.second:
            layerWidgets = buildSecondLayer(layout, keyWidth);
            break;
          case VirtualKeyboardLayer.third:
            layerWidgets = buildThirdLayer(layout, keyWidth);
            break;
        }

        return Column(
          children: [
            for (final row in layerWidgets)
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: row,
              ),
          ],
        );
      },
    );
  }

  void onLetter(String char) {
    widget.onCharacter(char);
    if (letterCase.value != Case.capslock) {
      letterCase.value = Case.lowercase;
    }
  }

  List<List<Widget>> buildFirstLayer(VirtualKeyboardLayout layout, double keyWidth, Case letterCase) {
    Widget letter(char) {
      if (letterCase != Case.lowercase) {
        char = char.toUpperCase();
      }
      return VirtualKeyboardKey(
        width: keyWidth,
        onTap: () => onLetter(char),
        child: Text(char),
      );
    }

    return [
      [for (String char in layout.first[0]) letter(char)],
      [for (String char in layout.first[1]) letter(char)],
      [
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          onTap: () {
            if (this.letterCase.value == Case.lowercase) {
              this.letterCase.value = Case.uppercase;
            } else {
              this.letterCase.value = Case.lowercase;
            }
          },
          onDoubleTap: () => this.letterCase.value = Case.capslock,
          child: Transform.rotate(
            angle: -90 * math.pi / 180,
            child: Icon(
              letterCase != Case.lowercase ? Icons.forward : Icons.forward_outlined,
              color: letterCase == Case.capslock ? Colors.blue : null,
            ),
          ),
        ),
        for (String char in layout.first[2]) letter(char),
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          repeatOnLongPress: true,
          onTap: () => widget.onKeyCode(KeyCode.backspace),
          child: const Icon(Icons.backspace_outlined),
        ),
      ],
      [
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          child: const Text(
            "?123",
            style: TextStyle(fontSize: 17),
          ),
          onTap: () => layer.value = VirtualKeyboardLayer.second,
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.first[3][0]),
          child: Text(layout.first[3][0]),
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
            onTap: () => widget.onCharacter(" "),
            child: const Text(" "),
          ),
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.first[3][1]),
          child: Text(layout.first[3][1]),
        ),
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          onTap: () => widget.onKeyCode(KeyCode.enter),
          child: const Icon(Icons.keyboard_return),
        ),
      ]
    ];
  }

  List<List<Widget>> buildSecondLayer(VirtualKeyboardLayout layout, double keyWidth) {
    return [
      [
        for (String char in layout.second[0])
          VirtualKeyboardKey(
            width: keyWidth,
            onTap: () => widget.onCharacter(char),
            child: Text(char),
          ),
      ],
      [
        for (String char in layout.second[1])
          VirtualKeyboardKey(
            width: keyWidth,
            onTap: () => widget.onCharacter(char),
            child: Text(char),
          ),
      ],
      [
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          onTap: () => layer.value = VirtualKeyboardLayer.third,
          child: const Text(
            "=\\<",
            style: TextStyle(fontSize: 17),
          ),
        ),
        for (String char in layout.second[2])
          VirtualKeyboardKey(
            width: keyWidth,
            onTap: () => widget.onCharacter(char),
            child: Text(char),
          ),
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          repeatOnLongPress: true,
          onTap: () => widget.onKeyCode(KeyCode.backspace),
          child: const Icon(Icons.backspace_outlined),
        ),
      ],
      [
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          child: const Text(
            "ABC",
            style: TextStyle(fontSize: 17),
          ),
          onTap: () => layer.value = VirtualKeyboardLayer.first,
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.second[3][0]),
          child: Text(layout.second[3][0]),
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.second[3][1]),
          child: Text(layout.second[3][1]),
        ),
        Expanded(
          child: VirtualKeyboardKey(
            width: double.infinity,
            popUpOnPress: false,
            onTap: () => widget.onCharacter(" "),
            child: const Text(" "),
          ),
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.second[3][2]),
          child: Text(layout.second[3][2]),
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.second[3][3]),
          child: Text(layout.second[3][3]),
        ),
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          onTap: () => widget.onKeyCode(KeyCode.enter),
          child: const Icon(Icons.keyboard_return),
        ),
      ]
    ];
  }

  List<List<Widget>> buildThirdLayer(VirtualKeyboardLayout layout, double keyWidth) {
    return [
      [
        for (String char in layout.third[0])
          VirtualKeyboardKey(
            width: keyWidth,
            onTap: () => widget.onCharacter(char),
            child: Text(char),
          ),
      ],
      [
        for (String char in layout.third[1])
          VirtualKeyboardKey(
            width: keyWidth,
            onTap: () => widget.onCharacter(char),
            child: Text(char),
          ),
      ],
      [
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          onTap: () => layer.value = VirtualKeyboardLayer.second,
          child: const Text(
            "?123",
            style: TextStyle(fontSize: 17),
          ),
        ),
        for (String char in layout.third[2])
          VirtualKeyboardKey(
            width: keyWidth,
            onTap: () => widget.onCharacter(char),
            child: Text(char),
          ),
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          repeatOnLongPress: true,
          onTap: () => widget.onKeyCode(KeyCode.backspace),
          child: const Icon(Icons.backspace_outlined),
        ),
      ],
      [
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          child: const Text(
            "ABC",
            style: TextStyle(fontSize: 17),
          ),
          onTap: () => layer.value = VirtualKeyboardLayer.first,
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.third[3][0]),
          child: Text(layout.third[3][0]),
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.third[3][1]),
          child: Text(layout.third[3][1]),
        ),
        Expanded(
          child: VirtualKeyboardKey(
            width: double.infinity,
            popUpOnPress: false,
            onTap: () => widget.onCharacter(" "),
            child: const Text(" "),
          ),
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.third[3][2]),
          child: Text(layout.third[3][2]),
        ),
        VirtualKeyboardKey(
          width: keyWidth,
          onTap: () => widget.onCharacter(layout.third[3][3]),
          child: Text(layout.third[3][3]),
        ),
        VirtualKeyboardKey(
          width: keyWidth * 1.5,
          popUpOnPress: false,
          onTap: () => widget.onKeyCode(KeyCode.enter),
          child: const Icon(Icons.keyboard_return),
        ),
      ]
    ];
  }
}
