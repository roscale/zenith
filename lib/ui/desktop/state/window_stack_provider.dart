import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:riverpod_annotation/riverpod_annotation.dart';

part '../../../generated/ui/desktop/state/window_stack_provider.freezed.dart';

part '../../../generated/ui/desktop/state/window_stack_provider.g.dart';

@Riverpod(keepAlive: true)
class WindowStack extends _$WindowStack {
  @override
  WindowStackState build() {
    return const WindowStackState(stack: []);
  }

  void set(Iterable<int> list) {
    state = WindowStackState(
      stack: [...list],
    );
  }

  void add(int viewId) {
    state = WindowStackState(
      stack: [
        ...state.stack,
        viewId,
      ],
    );
  }

  void remove(int viewId) {
    state = WindowStackState(
      stack: [
        for (int id in state.stack)
          if (id != viewId) id
      ],
    );
  }

  void raise(int viewId) {
    remove(viewId);
    add(viewId);
  }

  void clear() {
    state = const WindowStackState(stack: []);
  }
}

@freezed
class WindowStackState with _$WindowStackState {
  const WindowStackState._();

  const factory WindowStackState({
    required List<int> stack,
  }) = _WindowStackState;

  Iterable<int> get windows => stack;
}
