import 'package:elinux_app/desktop_state.dart';
import 'package:elinux_app/title_bar.dart';
import 'package:elinux_app/window_state.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class Window extends StatefulWidget {
  final int initialWidth;
  final int initialHeight;
  final int textureId;
  final int viewPtr;

  const Window({
    Key? key,
    required this.textureId,
    required this.viewPtr,
    required this.initialWidth,
    required this.initialHeight,
  }) : super(key: key);

  @override
  _WindowState createState() => _WindowState();
}

class _WindowState extends State<Window> with TickerProviderStateMixin {
  late WindowState windowState;

  late final AnimationController _controller = AnimationController(
    duration: const Duration(milliseconds: 300),
    vsync: this,
  )..repeat(reverse: true);
  late final Animation<double> _animation = CurvedAnimation(
    parent: _controller,
    curve: Curves.fastOutSlowIn,
  );

  double scale = 1.0;

  @override
  void dispose() {
    super.dispose();
    _controller.dispose();
  }

  @override
  void initState() {
    super.initState();
    windowState = WindowState(
        "Window",
        Rect.fromLTWH(
          100,
          100,
          widget.initialWidth.toDouble(),
          widget.initialHeight.toDouble(),
        ),
        widget.textureId);
  }

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider.value(
      value: windowState,
      child: Builder(builder: (context) {
        var windowState = context.watch<WindowState>();

        return Positioned(
          child: GestureDetector(
            onPanDown: (_) =>
                context.read<DesktopState>().activateWindow(widget),
            child: TweenAnimationBuilder(
              duration: const Duration(milliseconds: 200),
              tween: Tween(begin: 0.0, end: 1.0),
              curve: Curves.easeOutCubic,
              builder: (BuildContext context, double value, Widget? child) {
                return Transform.scale(
                  scale: value,
                  child: Material(
                    elevation: 20,
                    child: SizedBox(
                      width: windowState.rect.width,
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.stretch,
                        children: [
                          const TitleBar(),
                          if (windowState.textureId != 0)
                            SizedBox(
                              width: windowState.rect.width,
                              height: windowState.rect.height,
                              child: Texture(
                                filterQuality: FilterQuality.none,
                                textureId: windowState.textureId,
                              ),
                            ),
                          if (windowState.textureId == 0)
                            SizedBox(
                              width: windowState.rect.width,
                              height: windowState.rect.height,
                              child: Container(color: Colors.red),
                            ),
                        ],
                      ),
                    ),
                  ),
                );
              },
            ),
          ),
          left: windowState.rect.left,
          top: windowState.rect.top,
          // width: windowState.rect.width,
          // height: windowState.rect.height,
        );
      }),
    );
  }
}
