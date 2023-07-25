import 'package:riverpod_annotation/riverpod_annotation.dart';

part '../../generated/util/state/ui_mode_state.g.dart';

enum UiMode {
  mobile,
  desktop,
}

@Riverpod(keepAlive: true)
class UiModeState extends _$UiModeState {
  @override
  UiMode build() => UiMode.desktop;
}
