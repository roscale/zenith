import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:zenith/ui/common/app_icon.dart';
import 'package:zenith/ui/common/state/desktop_entries.dart';
import 'package:zenith/ui/mobile/state/app_drawer_state.dart';

final _appWidgetsProvider = Provider<List<Widget>>((ref) {
  String filter = ref.watch(appDrawerFilter).toLowerCase();

  return ref.watch(appDrawerDesktopEntriesProvider).when(
        data: (List<LocalizedDesktopEntry> desktopEntries) {
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

          return filtered
              .map(
                (desktopEntry) => Padding(
                  padding: const EdgeInsets.all(0.0),
                  child: AppEntry(desktopEntry: desktopEntry),
                ),
              )
              .toList();
        },
        error: (_, __) => [],
        loading: () => [],
      );
});

final appDrawerFilter = StateProvider((ref) => "");

class AppGrid extends ConsumerStatefulWidget {
  final ScrollController scrollController;

  const AppGrid({super.key, required this.scrollController});

  @override
  ConsumerState<AppGrid> createState() => _AppGridState();
}

class _AppGridState extends ConsumerState<AppGrid> {
  @override
  Widget build(BuildContext context) {
    final widgets = ref.watch(_appWidgetsProvider);
    bool dragging = ref.watch(appDrawerStateProvider.select((value) => value.dragging));

    return GridView.builder(
      controller: widget.scrollController,
      physics: dragging ? const NeverScrollableScrollPhysics() : const ClampingScrollPhysics(),
      itemCount: widgets.length,
      gridDelegate: const SliverGridDelegateWithMaxCrossAxisExtent(maxCrossAxisExtent: 100),
      itemBuilder: (BuildContext context, int index) => widgets[index],
    );
  }
}

class AppEntry extends ConsumerWidget {
  final LocalizedDesktopEntry desktopEntry;

  const AppEntry({
    Key? key,
    required this.desktopEntry,
  }) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return InkWell(
      onTap: () async {
        String? exec = desktopEntry.entries[DesktopEntryKey.exec.string];
        if (exec == null) {
          return;
        }

        // FIXME
        exec = exec.replaceAll(RegExp(r'( %.?)'), '');
        debugPrint(exec);

        final bool terminal = desktopEntry.entries[DesktopEntryKey.terminal.string]?.getBoolean() ?? false;

        try {
          if (terminal) {
            await Process.start('kgx', ['-e', exec]);
          } else {
            await Process.start('/bin/sh', ['-c', exec]);
          }
        } on ProcessException catch (e) {
          stderr.write(e.toString());
        }

        ref.read(appDrawerStateProvider.notifier).update((state) => state.copyWith(closePanel: Object()));
      },
      child: Column(
        children: [
          Expanded(
            child: Padding(
              padding: const EdgeInsets.all(8.0),
              child: AppIconByPath(
                path: desktopEntry.entries[DesktopEntryKey.icon.string],
              ),
            ),
          ),
          Text(
            desktopEntry.entries[DesktopEntryKey.name.string] ?? '',
            maxLines: 1,
            overflow: TextOverflow.ellipsis,
            style: const TextStyle(
              color: Colors.white,
            ),
          ),
        ],
      ),
    );
  }
}
