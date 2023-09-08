import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_portal/flutter_portal.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/desktop/app_drawer/app_drawer_button.dart';
import 'package:zenith/ui/desktop/task_bar.dart';
import 'package:zenith/ui/desktop/window_manager.dart';

class DesktopUi extends ConsumerStatefulWidget {
  const DesktopUi({Key? key}) : super(key: key);

  @override
  ConsumerState<DesktopUi> createState() => _DesktopUiState();
}

/// FIXME:
/// This shouldn't be necessary but I have problems with keyboard input.
/// When the super key is pressed, the state of the keyboard says that both meta left and meta right
/// are pressed instead of super left or super right.
/// It's even more confusing because the linux kernel itself only has a definition for the meta key,
/// not the super key, and it's considered pressed when the super key is pressed.
class SingleLogicalKeyReleaseActivator extends ShortcutActivator {
  LogicalKeyboardKey key;

  SingleLogicalKeyReleaseActivator(this.key);

  @override
  bool accepts(RawKeyEvent event, RawKeyboard state) {
    if (event is! RawKeyUpEvent) {
      return false;
    }
    return event.logicalKey == key;
  }

  @override
  String debugDescribeKeys() => "";

  @override
  Iterable<LogicalKeyboardKey>? get triggers => [key];
}

class ToggleAppDrawerIntent extends Intent {}

class _DesktopUiState extends ConsumerState<DesktopUi> {
  final backgroundFocusNode = FocusNode();

  @override
  Widget build(BuildContext context) {
    return Shortcuts(
      shortcuts: {
        SingleLogicalKeyReleaseActivator(LogicalKeyboardKey.superKey): ToggleAppDrawerIntent(),
      },
      child: Actions(
        actions: {
          ToggleAppDrawerIntent: CallbackAction(onInvoke: (_) {
            ref.read(appDrawerVisibleProvider.notifier).update((visible) => !visible);
            return null;
          }),
        },
        child: FocusScope(
          autofocus: true,
          child: Portal(
            child: Stack(
              clipBehavior: Clip.none,
              fit: StackFit.expand,
              children: [
                Focus(
                  focusNode: backgroundFocusNode,
                  child: GestureDetector(
                    onTapDown: (_) {
                      backgroundFocusNode.requestFocus();
                    },
                    child: Image.asset("assets/images/background.jpg", fit: BoxFit.cover),
                  ),
                ),
                Overlay(
                  initialEntries: [
                    OverlayEntry(
                      builder: (_) => const Column(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          Expanded(
                            child: WindowManager(),
                          ),
                          TaskBar(),
                        ],
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  @override
  void dispose() {
    backgroundFocusNode.dispose();
    super.dispose();
  }
}
