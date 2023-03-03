import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freezed_annotation/freezed_annotation.dart';

part 'window_stack_notifier_provider.freezed.dart';

class WindowStackNotifierProvider extends StateNotifier<WindowStack> {
  WindowStackNotifierProvider() : super(const WindowStack(stack: []));

  void set(Iterable<int> list) {
    state = WindowStack(
      stack: [...list],
    );
  }

  void add(int viewId) {
    state = WindowStack(
      stack: [
        ...state.stack,
        viewId,
      ],
    );
  }

  void remove(int viewId) {
    state = WindowStack(
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
    state = const WindowStack(stack: []);
  }
}

@freezed
class WindowStack with _$WindowStack {
  const WindowStack._();

  const factory WindowStack({
    required List<int> stack,
  }) = _WindowStack;

  Iterable<int> get windows => stack;
}
