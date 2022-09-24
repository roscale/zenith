import 'package:flutter_riverpod/flutter_riverpod.dart';

class StateNotifierList<T> extends StateNotifier<List<T>> {
  StateNotifierList() : super(const []);

  void add(T element) {
    state = [...state, element];
  }

  void insert(int index, T element) {
    final copy = [...state];
    copy.insert(index, element);
    state = copy;
  }

  void remove(T element) {
    state = [
      for (final e in state)
        if (e != element) e
    ];
  }

  T removeAt(int index) {
    final copy = [...state];
    final toReturn = copy.removeAt(index);
    state = copy;
    return toReturn;
  }

  void operator []=(int index, T element) {
    state = [
      for (int i = 0; i < state.length; i++)
        if (i == index) element else state[i]
    ];
  }

  void clear() {
    state = [];
  }
}
