import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:freezed_annotation/freezed_annotation.dart';
import 'package:jovial_svg/jovial_svg.dart';
import 'package:riverpod_annotation/riverpod_annotation.dart';
import 'package:zenith/system_ui/app_drawer/app_drawer.dart';

part 'app_drawer_state.freezed.dart';
part 'app_drawer_state.g.dart';

final appDrawerStateProvider = StateProvider(
  (ref) => AppDrawerState(
    /// The drawer is in a state where a drag can be engaged, e.g. the drawer is fully scrolled at the top.
    draggable: false,

    /// The user is actively dragging the drawer.
    dragging: false,

    /// You can interact with the drawer and taps won't just go though it.
    interactable: false,

    /// The finger velocity when the drag ends.
    dragVelocity: 0,

    /// 0 means the drawer is fully open.
    /// Equal to slideDistance when it's fully closed.
    offset: 300,

    /// The amount the user has to drag to open the drawer.
    slideDistance: 300,

    /// The actual drawer widget that is inserted into the Overlay.
    overlayEntry: OverlayEntry(builder: (_) => const AppDrawer()),

    /// Event to notify the drawer to initiate the closing animations.
    /// Just assigning a new Object() will do the trick because 2 different Object instances will always be unequal.
    closePanel: Object(),
  ),
);

@freezed
class AppDrawerState with _$AppDrawerState {
  const factory AppDrawerState({
    required bool draggable,
    required bool dragging,
    required bool interactable,
    required double dragVelocity,
    required double offset,
    required double slideDistance,
    required OverlayEntry overlayEntry,
    required Object closePanel,
  }) = _AppDrawerState;
}

@Riverpod(keepAlive: true)
Future<ScalableImage> fileToScalableImage(FileToScalableImageRef ref, String path) async {
  String svg = await File(path).readAsString();
  return ScalableImage.fromSvgString(svg);
}

@Riverpod(keepAlive: true)
Future<List<LocalizedDesktopEntry>> desktopEntries(DesktopEntriesRef ref) {
  return _getAllDesktopEntriesLocalized();
}

@Riverpod(keepAlive: true)
Future<FreedesktopIconTheme> defaultIconTheme(DefaultIconThemeRef ref) {
  return FreedesktopIconTheme.load('Adwaita');
}

Stream<File> _getAllDesktopEntryFiles() async* {
  for (String dir in getAppBaseDirectories()) {
    Directory appDir = Directory(dir);
    if (await appDir.exists()) {
      await for (FileSystemEntity entity in appDir.list()) {
        if (entity is File) {
          yield entity.absolute;
        }
      }
    }
  }
}

Stream<DesktopEntry> _getAllDesktopEntries() async* {
  await for (File file in _getAllDesktopEntryFiles()) {
    final desktopEntry = DesktopEntry.parse(await file.readAsString());
    if (!desktopEntry.isHidden()) {
      yield desktopEntry;
    }
  }
}

Future<List<LocalizedDesktopEntry>> _getAllDesktopEntriesLocalized() async {
  List<LocalizedDesktopEntry> localizedDesktopEntries =
      await _getAllDesktopEntries().map((event) => event.localize(lang: 'en')).toList();

  localizedDesktopEntries.sort((a, b) {
    String aName = a.entries[DesktopEntryKey.name.string]!;
    String bName = b.entries[DesktopEntryKey.name.string]!;
    return aName.toLowerCase().compareTo(bName.toLowerCase());
  });

  return localizedDesktopEntries;
}
