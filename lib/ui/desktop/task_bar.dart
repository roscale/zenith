import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/desktop/app_drawer/app_drawer_button.dart';

final taskBarHeightProvider = Provider<double>((ref) => 50.0);

class TaskBar extends ConsumerWidget {
  const TaskBar({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Material(
      color: Colors.white38,
      child: SizedBox(
        height: ref.watch(taskBarHeightProvider),
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          mainAxisAlignment: MainAxisAlignment.center,
          children: const [
            AppDrawerButton(),

            // SizedBox(
            //   width: 100,
            //   child: TextField(
            //     keyboardType: TextInputType.multiline,
            //     maxLines: null,
            //     onSubmitted: (s) {
            //       print("SUBMITTED $s");
            //     },
            //   ),
            // ),
          ],
        ),
      ),
    );
  }
}
