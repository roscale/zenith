import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:freedesktop_desktop_entry/freedesktop_desktop_entry.dart';
import 'package:zenith/ui/common/app_drawer.dart';
import 'package:zenith/ui/common/app_icon.dart';
import 'package:zenith/ui/desktop/app_drawer/app_drawer_button.dart';
import 'package:zenith/util/app_launch.dart';

final _appWidgetsProvider = Provider<List<Widget>>((ref) {
  var desktopEntries = ref.watch(appDrawerFilteredDesktopEntriesProvider);
  if (!desktopEntries.hasValue) {
    return [];
  }
  return [
    for (var desktopEntry in desktopEntries.value!) AppEntry(desktopEntry: desktopEntry),
  ];
});

class AppGrid extends ConsumerStatefulWidget {
  const AppGrid({super.key});

  @override
  ConsumerState<AppGrid> createState() => _AppGridState();
}

class _AppGridState extends ConsumerState<AppGrid> {
  @override
  Widget build(BuildContext context) {
    final widgets = ref.watch(_appWidgetsProvider);

    return Material(
      color: Colors.transparent,
      child: GridView.builder(
        itemCount: widgets.length,
        padding: const EdgeInsets.all(10),
        gridDelegate: const SliverGridDelegateWithMaxCrossAxisExtent(
          maxCrossAxisExtent: 90,
          childAspectRatio: 0.65,
        ),
        itemBuilder: (BuildContext context, int index) => widgets[index],
      ),
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
        if (await launchDesktopEntry(desktopEntry.desktopEntry)) {
          ref.read(appDrawerVisibleProvider.notifier).state = false;
        }
      },
      child: Column(
        children: [
          LayoutBuilder(
            builder: (BuildContext context, BoxConstraints constraints) {
              return ConstrainedBox(
                constraints: constraints.copyWith(
                  minHeight: constraints.maxWidth,
                  maxHeight: constraints.maxWidth,
                ),
                child: Padding(
                  padding: const EdgeInsets.all(10.0),
                  child: AppIconByPath(
                    path: desktopEntry.entries[DesktopEntryKey.icon.string],
                  ),
                ),
              );
            },
          ),
          Expanded(
            child: Text(
              desktopEntry.entries[DesktopEntryKey.name.string] ?? '',
              maxLines: 2,
              overflow: TextOverflow.ellipsis,
              textAlign: TextAlign.center,
              style: const TextStyle(
                color: Colors.black,
                fontWeight: FontWeight.w700,
              ),
            ),
          ),
        ],
      ),
    );
  }
}
