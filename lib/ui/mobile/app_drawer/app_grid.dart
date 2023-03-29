import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:zenith/ui/common/app_icon.dart';
import 'package:zenith/ui/common/state/desktop_entries.dart';
import 'package:zenith/ui/mobile/state/app_drawer_state.dart';

final _appWidgetCacheProvider = Provider<List<Widget>>((ref) {
  return ref.watch(appDrawerDesktopEntriesProvider).when(
        data: (List<LocalizedDesktopEntry> desktopEntries) {
          return desktopEntries
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

class AppGrid extends ConsumerStatefulWidget {
  final ScrollController scrollController;

  const AppGrid({super.key, required this.scrollController});

  @override
  ConsumerState<AppGrid> createState() => _AppGridState();
}

class _AppGridState extends ConsumerState<AppGrid> {
  @override
  Widget build(BuildContext context) {
    final widgets = ref.watch(_appWidgetCacheProvider);
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
