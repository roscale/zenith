import 'dart:math' as math;

import 'package:collection/collection.dart';
import 'package:flutter/material.dart';

class ChangeNotifierList<E> extends DelegatingList<E> with ChangeNotifier {
  ChangeNotifierList() : super(<E>[]);

  @override
  void sort([int Function(E, E)? compare]) {
    super.sort(compare);
    notifyListeners();
  }

  @override
  void shuffle([math.Random? random]) {
    super.shuffle(random);
    notifyListeners();
  }

  @override
  void setRange(int start, int end, Iterable<E> iterable, [int skipCount = 0]) {
    super.setRange(start, end, iterable, skipCount);
    notifyListeners();
  }

  @override
  void setAll(int index, Iterable<E> iterable) {
    super.setAll(index, iterable);
    notifyListeners();
  }

  @override
  void retainWhere(bool Function(E) test) {
    super.retainWhere(test);
    notifyListeners();
  }

  @override
  void replaceRange(int start, int end, Iterable<E> iterable) {
    super.replaceRange(start, end, iterable);
    notifyListeners();
  }

  @override
  void removeWhere(bool Function(E) test) {
    super.removeWhere(test);
    notifyListeners();
  }

  @override
  void removeRange(int start, int end) {
    super.removeRange(start, end);
    notifyListeners();
  }

  @override
  E removeLast() {
    E value = super.removeLast();
    notifyListeners();
    return value;
  }

  @override
  E removeAt(int index) {
    E value = super.removeAt(index);
    notifyListeners();
    return value;
  }

  @override
  bool remove(Object? value) {
    bool removed = super.remove(value);
    notifyListeners();
    return removed;
  }

  @override
  void insertAll(int index, Iterable<E> iterable) {
    super.insertAll(index, iterable);
    notifyListeners();
  }

  @override
  void insert(int index, E element) {
    super.insert(index, element);
    notifyListeners();
  }

  @override
  void fillRange(int start, int end, [E? fillValue]) {
    super.fillRange(start, end, fillValue);
    notifyListeners();
  }

  @override
  void clear() {
    super.clear();
    notifyListeners();
  }

  @override
  void addAll(Iterable<E> iterable) {
    super.addAll(iterable);
    notifyListeners();
  }

  @override
  void add(E value) {
    super.add(value);
    notifyListeners();
  }

  @override
  void operator []=(int index, E value) {
    super[index] = value;
    notifyListeners();
  }
}
