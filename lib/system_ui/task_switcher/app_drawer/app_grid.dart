import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart' as freedesktop;
import 'package:jovial_svg/jovial_svg.dart';
import 'package:zenith/state/app_drawer_state.dart';

class AppGrid extends ConsumerWidget {
  final ScrollController scrollController;

  const AppGrid({super.key, required this.scrollController});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    bool dragging = ref.watch(appDrawerStateProvider.select((value) => value.dragging));

    return ref.watch(desktopEntriesProvider).when(
          data: (desktopEntries) => _buildList(desktopEntries, dragging),
          error: (_, __) => const SizedBox(),
          loading: () => const SizedBox(),
        );
  }

  Widget _buildList(List<LocalizedDesktopEntry> desktopEntries, bool dragging) {
    return GridView.builder(
      controller: scrollController,
      physics: dragging ? const NeverScrollableScrollPhysics() : const ClampingScrollPhysics(),
      itemCount: desktopEntries.length,
      gridDelegate: const SliverGridDelegateWithMaxCrossAxisExtent(
        maxCrossAxisExtent: 150,
      ),
      itemBuilder: (BuildContext context, int index) {
        return Padding(
          padding: const EdgeInsets.all(8.0),
          child: AppEntry(desktopEntry: desktopEntries[index]),
        );
      },
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

        final bool terminal = desktopEntry.entries[DesktopEntryKey.terminal.string]?.getBoolean() ?? false;

        if (terminal) {
          await Process.start('konsole', ['-e', exec]);
        } else {
          await Process.start('/bin/sh', ['-c', exec]);
        }

        ref.read(appDrawerStateProvider.notifier).update((state) => state.copyWith(closePanel: Object()));
      },
      child: Column(
        children: [
          Expanded(
            child: AppIcon(
              icon: desktopEntry.entries[DesktopEntryKey.icon.string],
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

class AppIcon extends StatelessWidget {
  final String? icon;

  const AppIcon({
    Key? key,
    required this.icon,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Consumer(
      builder: (_, WidgetRef ref, __) {
        if (icon == null) {
          return const SizedBox();
        }

        return ref.watch(defaultIconThemeProvider).when(
              data: (iconTheme) {
                return Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: _buildIcon(iconTheme, icon!),
                );
              },
              error: (_, __) => const SizedBox(),
              loading: () => const SizedBox(),
            );
      },
    );
  }

  Widget _buildIcon(freedesktop.IconTheme iconTheme, String icon) {
    return LayoutBuilder(
      builder: (BuildContext context, BoxConstraints constraints) {
        File? file = iconTheme.findIcon(
          name: icon,
          size: constraints.biggest.shortestSide.floor(),
          extensions: {'svg', 'png'},
        );

        if (file == null) {
          return const SizedBox();
        }

        if (file.path.endsWith('.svg')) {
          return Consumer(builder: (BuildContext context, WidgetRef ref, Widget? child) {
            return ref.watch(fileToScalableImageProvider(file.path)).when(
              data: (ScalableImage si) {
                return SizedBox.expand(
                  child: ScalableImageWidget(
                    si: si,
                  ),
                );
              },
              error: (e, st) {
                return const SizedBox();
              },
              loading: () {
                return const SizedBox();
              },
            );
          });
        }

        return Image.file(
          file,
          filterQuality: FilterQuality.medium,
          fit: BoxFit.fill,
        );
      },
    );
  }
}
