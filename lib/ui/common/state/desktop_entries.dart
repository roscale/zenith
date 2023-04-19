import 'dart:io';

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:jovial_svg/jovial_svg.dart';

final installedDesktopEntriesProvider = FutureProvider<Map<String, DesktopEntry>>((ref) async {
  return await parseAllInstalledDesktopFiles();
});

final localizedDesktopEntriesProvider = FutureProvider<Map<String, LocalizedDesktopEntry>>((ref) async {
  final desktopEntries = await ref.watch(installedDesktopEntriesProvider.future);
  return desktopEntries.map((key, value) => MapEntry(key, value.localize(lang: 'en')));
});

final appDrawerDesktopEntriesProvider = FutureProvider<Iterable<LocalizedDesktopEntry>>((ref) async {
  final localizedDesktopEntries = await ref.watch(localizedDesktopEntriesProvider.future);
  return localizedDesktopEntries.values.where((element) => !element.desktopEntry.isHidden());
});

final defaultIconThemeProvider = FutureProvider<FreedesktopIconTheme>((ref) {
  return FreedesktopIconTheme.load('Adwaita');
});

final fileToScalableImageProvider = FutureProvider.family<ScalableImage, String>((ref, String path) async {
  String svg = await File(path).readAsString();
  return ScalableImage.fromSvgString(svg);
});
