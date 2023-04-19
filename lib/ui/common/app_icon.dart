import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:jovial_svg/jovial_svg.dart';
import 'package:zenith/ui/common/state/desktop_entries.dart';

class AppIconByPath extends StatelessWidget {
  final String? path;

  const AppIconByPath({
    Key? key,
    required this.path,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Consumer(
      builder: (_, WidgetRef ref, __) {
        if (path == null) {
          return const SizedBox();
        }

        return ref.watch(defaultIconThemeProvider).when(
              data: (iconTheme) => _buildIcon(iconTheme, path!),
              error: (_, __) => const SizedBox(),
              loading: () => const SizedBox(),
            );
      },
    );
  }

  Widget _buildIcon(FreedesktopIconTheme iconTheme, String icon) {
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
            return ref.watch(fileToScalableImageProvider(file.absolute.path)).when(
              data: (ScalableImage si) {
                return SizedBox.expand(
                  child: ScalableImageWidget(
                    si: si,
                  ),
                );
              },
              error: (Object error, StackTrace stackTrace) {
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
          fit: BoxFit.contain,
        );
      },
    );
  }
}

class AppIconById extends ConsumerWidget {
  final String id;

  const AppIconById({
    super.key,
    required this.id,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return ref.watch(localizedDesktopEntriesProvider).maybeWhen(
          data: (Map<String, LocalizedDesktopEntry> desktopEntries) {
            LocalizedDesktopEntry? entry = desktopEntries[id];
            if (entry == null) {
              return const SizedBox();
            }
            String? iconPath = entry.entries[DesktopEntryKey.icon.string];
            if (iconPath == null) {
              return const SizedBox();
            }
            return AppIconByPath(path: iconPath);
          },
          orElse: () => const SizedBox(),
        );
  }
}
