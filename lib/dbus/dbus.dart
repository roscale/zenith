import 'package:dbus/dbus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final dbusSystemBusProvider = Provider((ref) {
  final client = DBusClient.system();
  ref.onDispose(() => client.close());
  return client;
});
