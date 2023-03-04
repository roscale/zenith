import 'package:flutter/cupertino.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/popup.dart';
import 'package:zenith/util/state_notifier_list.dart';

final popupStackGlobalKey = Provider((ref) => GlobalKey());

final popupStackChildren = StateNotifierProvider<StateNotifierList<int>, List<int>>((ref) {
  return StateNotifierList<int>();
});

class PopupStack extends ConsumerWidget {
  const PopupStack({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Stack(
      key: ref.watch(popupStackGlobalKey),
      children: [
        for (int viewId in ref.watch(popupStackChildren)) ref.watch(popupWidget(viewId)),
      ],
    );
  }
}
