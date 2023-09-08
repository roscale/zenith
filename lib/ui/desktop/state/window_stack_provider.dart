import 'dart:ui';

import 'package:fast_immutable_collections/fast_immutable_collections.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:riverpod_annotation/riverpod_annotation.dart';

part '../../../generated/ui/desktop/state/window_stack_provider.freezed.dart';
part '../../../generated/ui/desktop/state/window_stack_provider.g.dart';

@Riverpod(keepAlive: true)
class WindowStack extends _$WindowStack {
  @override
  WindowStackState build() {
    return WindowStackState(
      stack: <int>[].lock,
      animateClosing: <int>{}.lock,
      desktopSize: Size.zero,
    );
  }

  void set(Iterable<int> list) {
    state = state.copyWith(
      stack: list.toIList(),
      animateClosing: <int>{}.lock,
    );
  }

  void add(int viewId) {
    assert(!state.stack.contains(viewId));
    state = state.copyWith(
      stack: state.stack.add(viewId),
    );
  }

  void close(int viewId) {
    assert(!state.animateClosing.contains(viewId));
    state = state.copyWith(
      animateClosing: state.animateClosing.add(viewId),
    );
  }

  void remove(int viewId) {
    assert(state.stack.contains(viewId));
    state = state.copyWith(
      stack: state.stack.remove(viewId),
      animateClosing: state.animateClosing.remove(viewId),
    );
  }

  void raise(int viewId) {
    remove(viewId);
    add(viewId);
  }

  void clear() {
    state = state.copyWith(
      stack: <int>[].lock,
      animateClosing: <int>{}.lock,
    );
  }

  void setDesktopSize(Size size) {
    state = state.copyWith(
      desktopSize: size,
    );
  }
}

@freezed
class WindowStackState with _$WindowStackState {
  const WindowStackState._();

  const factory WindowStackState({
    required IList<int> stack,
    required ISet<int> animateClosing,
    required Size desktopSize,
  }) = _WindowStackState;

  Iterable<int> get windows => stack;
}
