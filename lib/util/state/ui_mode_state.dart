import 'package:flutter_riverpod/flutter_riverpod.dart';

enum UiMode {
  mobile,
  desktop,
}

final uiModeStateProvider = StateProvider<UiMode>((ref) {
  return UiMode.desktop;
});
