import 'package:zenith/desktop_state.dart';
import 'package:zenith/title_bar.dart';
import 'package:zenith/window_state.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class Window extends StatelessWidget {
  final int initialWidth;
  final int initialHeight;
  final int viewId;
  final int surfacePtr;
  final GlobalKey frameGlobalKey = GlobalKey();

  late final WindowState windowState = WindowState(
    "Window",
    Rect.fromLTWH(
      100,
      100,
      initialWidth.toDouble(),
      initialHeight.toDouble(),
    ),
  )..activate();

  Window({
    required this.viewId,
    required this.surfacePtr,
    required this.initialWidth,
    required this.initialHeight,
  }) : super(key: GlobalKey()) {
    print("instantiate window");

    WidgetsBinding.instance!.addPostFrameCallback((_) {
      windowState.animateOpening();
    });
  }

  WindowState getWindowState() {
    return windowState;
  }

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider.value(
      value: windowState,
      child: Builder(builder: (_) {
        return _PointerListener(
          child: _WindowAnimations(
            child: WindowFrame(
              frameGlobalKey: frameGlobalKey,
              viewId: viewId,
            ),
          ),
        );
      }),
    );
  }
}

// class LocalWindowState extends State<Window> {
//   @override
//   void initState() {
//     super.initState();
//     // Start window animations when this widget is created.
//     WidgetsBinding.instance!.addPostFrameCallback((_) {
//       windowState.animateOpening();
//     });
//   }
//
//   @override
//   Widget build(BuildContext context) {
//     return
//   }
// }

class _PointerListener extends StatelessWidget {
  final Widget child;

  const _PointerListener({required this.child});

  @override
  Widget build(BuildContext context) {
    var rect = context.select((WindowState state) => state.rect);
    var isClosing = context.select((WindowState state) => state.isClosing);

    return Positioned(
      left: rect.left.roundToDouble(),
      top: rect.top.roundToDouble(),
      child: IgnorePointer(
        ignoring: isClosing,
        child: Listener(
          onPointerDown: (_) {
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
    var shadowBlurRadius = context.select((WindowState state) => state.shadowBlurRadius);

    return AnimatedOpacity(
      curve: Curves.linearToEaseOut,
      opacity: opacity,
      duration: const Duration(milliseconds: 200),
      child: AnimatedScale(
        curve: Curves.linearToEaseOut,
        scale: scale,
        duration: const Duration(milliseconds: 200),
        onEnd: () {
          var windowState = context.read<WindowState>();
          if (windowState.isClosing) {
            windowState.windowClosed.complete();
          }
        },
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 100),
          // decoration: CustomBoxDecoration(
          //   borderRadius: const BorderRadius.all(Radius.circular(10)),
          //   boxShadow: [
          //     BoxShadow(
          //       // spreadRadius: -10,
          //       blurRadius: shadowBlurRadius,
          //     )
          //   ],
          // ),
          child: child,
        ),
      ),
    );
  }
}

class WindowFrame extends StatelessWidget {
  final int viewId;
  final GlobalKey frameGlobalKey;

  const WindowFrame({Key? key, required this.viewId, required this.frameGlobalKey}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var size = context.select((WindowState state) => state.rect.size);

    return ClipRRect(
      borderRadius: const BorderRadius.all(Radius.circular(10)),
      // clipBehavior: Clip.antiAliasWithSaveLayer,
      child: Material(
        type: MaterialType.transparency,
        child: SizedBox(
          width: size.width,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              const TitleBar(),
              SizedBox(
                width: size.width,
                height: size.height,
                child: Listener(
                  onPointerDown: pointerMoved,
                  onPointerUp: pointerMoved,
                  onPointerHover: pointerMoved,
                  onPointerMove: pointerMoved,
                  child: Texture(
                    key: frameGlobalKey,
                    filterQuality: FilterQuality.none,
                    textureId: viewId,
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
        "view_id": viewId,
      },
    );
  }
}
