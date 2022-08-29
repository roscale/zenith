enum VirtualKeyboardLayer {
  first,
  second,
  third,
}

class VirtualKeyboardLayout {
  List<List<String>> first;
  List<List<String>> second;
  List<List<String>> third;

  VirtualKeyboardLayout({
    required this.first,
    required this.second,
    required this.third,
  });
}

enum Case {
  lowercase,
  uppercase,
  capslock,
}

final layouts = {
  'en': VirtualKeyboardLayout(
    first: [
      ['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'],
      ['a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'],
      ['z', 'x', 'c', 'v', 'b', 'n', 'm'],
      [',', '.'],
    ],
    second: [
      ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0'],
      ['@', '#', '\$', '%', '&', '-', '+', '(', ')'],
      ['*', '"', '\'', ':', ';', '!', '?'],
      [',', '_', '/', '.'],
    ],
    third: [
      ['~', '`', '|', '•', '√', 'π', '÷', '×', '¶', '∆'],
      ['£', '€', '¢', '¥', '^', '°', '=', '{', '}'],
      ['\\', '©', '®', '™', '℅', '[', ']'],
      [',', '<', '>', '.'],
    ],
  )
};
