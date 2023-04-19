import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:zenith/ui/common/state/desktop_entries.dart';

final appDrawerFilterProvider = StateProvider((ref) => "");

final appDrawerFilteredDesktopEntriesProvider = FutureProvider((ref) async {
  String filter = ref.watch(appDrawerFilterProvider);
  var desktopEntries = await ref.watch(appDrawerDesktopEntriesProvider.future);

  var filtered = <LocalizedDesktopEntry>[];
  var desktopEntriesSet = Set<LocalizedDesktopEntry>.from(desktopEntries);

  int byNames(LocalizedDesktopEntry a, LocalizedDesktopEntry b) {
    String aName = a.entries[DesktopEntryKey.name.string]!;
    String bName = b.entries[DesktopEntryKey.name.string]!;
    return aName.toLowerCase().compareTo(bName.toLowerCase());
  }

  var nameMatched = desktopEntriesSet.where((d) {
    String name = d.entries[DesktopEntryKey.name.string]!.toLowerCase();
    return name.startsWith(filter);
  }).toList()
    ..sort(byNames);

  filtered.addAll(nameMatched);
  desktopEntriesSet.removeAll(nameMatched);

  var namePartsMatched = desktopEntriesSet.where((d) {
    String name = d.entries[DesktopEntryKey.name.string]!.toLowerCase();
    List<String> nameParts = name.split(" ");
    return nameParts.any((part) => part.startsWith(filter));
  }).toList()
    ..sort(byNames);

  filtered.addAll(namePartsMatched);
  desktopEntriesSet.removeAll(namePartsMatched);

  var keywordsMatched = desktopEntriesSet.where((d) {
    List<String>? keywords = d.entries[DesktopEntryKey.keywords.string]?.getStringList();
    if (keywords == null) {
      return false;
    }
    for (String kw in keywords) {
      kw.toLowerCase();
    }
    return keywords.any((kw) => kw.startsWith(filter));
  }).toList()
    ..sort(byNames);

  filtered.addAll(keywordsMatched);

  return filtered;
});
