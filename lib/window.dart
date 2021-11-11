import 'package:elinux_app/custom_box_decoration.dart';
import 'package:elinux_app/desktop_state.dart';
import 'package:elinux_app/title_bar.dart';
import 'package:elinux_app/window_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class Window extends StatefulWidget {
  final int initialWidth;
  final int initialHeight;
  final int textureId;
  final int viewPtr;

  Window({
    required this.textureId,
    required this.viewPtr,
    required this.initialWidth,
    required this.initialHeight,
  }) : super(key: GlobalKey<LocalWindowState>());

  @override
  LocalWindowState createState() => LocalWindowState();

  WindowState getWindowState() {
    return (key! as GlobalKey<LocalWindowState>).currentState!.windowState;
  }
}

class LocalWindowState extends State<Window> with TickerProviderStateMixin {
  late var windowState = WindowState(
    "Window",
    Rect.fromLTWH(
      100,
      100,
      widget.initialWidth.toDouble(),
      widget.initialHeight.toDouble(),
    ),
    widget.textureId,
  )..activate();

  @override
  void initState() {
    super.initState();
    // Start window animations when this widget is created.
    WidgetsBinding.instance!.addPostFrameCallback((_) {
      windowState.animateOpening();
    });
  }

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider.value(
      value: windowState,
      child: Builder(builder: (_) {
        return _GestureDetector(
          child: _WindowAnimations(
            child: _WindowFrame(
              viewPtr: widget.viewPtr,
              textureId: widget.textureId,
            ),
          ),
        );
      }),
    );
  }
}

class _GestureDetector extends StatelessWidget {
  final Widget child;

  const _GestureDetector({required this.child});

  @override
  Widget build(BuildContext context) {
    var rect = context.select((WindowState state) => state.rect);
    var isClosing = context.select((WindowState state) => state.isClosing);

    return Positioned(
      left: rect.left,
      top: rect.top,
      child: IgnorePointer(
        ignoring: isClosing,
        child: GestureDetector(
          onPanDown: (_) {
            var windowWidget = context.findAncestorWidgetOfExactType<Window>()!;
            context.read<DesktopState>().activateWindow(windowWidget);
          },
          child: child,
        ),
      ),
    );
  }
}

class _WindowAnimations extends StatelessWidget {
  final Widget child;

  const _WindowAnimations({required this.child});

  @override
  Widget build(BuildContext context) {
    var opacity = context.select((WindowState state) => state.opacity);
    var scale = context.select((WindowState state) => state.scale);

    return AnimatedOpacity(
      curve: Curves.linearToEaseOut,
      opacity: opacity,
      duration: const Duration(milliseconds: 200),
      child: AnimatedScale(
        curve: Curves.linearToEaseOut,
        scale: scale,
        duration: const Duration(milliseconds: 200),
        onEnd: context.read<WindowState>().windowClosed.complete,
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 100),
          // decoration: CustomBoxDecoration(
          //   borderRadius: const BorderRadius.all(Radius.circular(10)),
          //   boxShadow: [
          //     BoxShadow(
          //       // spreadRadius: -10,
          //       blurRadius: windowState.shadowBlurRadius,
          //     )
          //   ],
          // ),
          child: child,
        ),
      ),
    );
  }
}

class _WindowFrame extends StatelessWidget {
  final int viewPtr;
  final int textureId;

  const _WindowFrame({required this.viewPtr, required this.textureId});

  @override
  Widget build(BuildContext context) {
    var rect = context.select((WindowState state) => state.rect);
    var textureId = context.select((WindowState state) => state.textureId);

    return ClipRRect(
      borderRadius: const BorderRadius.all(Radius.circular(10)),
      // clipBehavior: Clip.antiAliasWithSaveLayer,
      child: Material(
        type: MaterialType.transparency,
        child: SizedBox(
          width: rect.width,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              const TitleBar(),
              SizedBox(
                width: rect.width,
                height: rect.height,
                child: Listener(
                  onPointerHover: (PointerHoverEvent event) => pointerMoved(event),
                  onPointerMove: (PointerMoveEvent event) => pointerMoved(event),
                  child: MouseRegion(
                    // onEnter: (PointerEnterEvent event) => print("enter"),
                    // onExit: (PointerExitEvent event) => print("exit"),
                    child: Texture(
                      filterQuality: FilterQuality.none,
                      textureId: textureId,
                    ),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  void pointerMoved(PointerEvent event) {
    DesktopState.platform.invokeMethod(
      "pointer_hover",
      {
        "x": event.localPosition.dx,
        "y": event.localPosition.dy,
        "view_ptr": viewPtr,
      },
    );
  }
}
