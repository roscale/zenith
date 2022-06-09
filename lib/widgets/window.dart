import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:zenith/platform_api.dart';
import 'package:zenith/state/window_state.dart';
import 'package:zenith/widgets/popup.dart';
import 'package:zenith/widgets/view_input_listener.dart';

class Window extends StatelessWidget {
  final WindowState state;

  Window(this.state) : super(key: GlobalKey());

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => state,
      child: const _PointerListener(
        child: _Surface(),
      ),
    );
  }
}

class _PointerListener extends StatefulWidget {
  final Widget child;

  const _PointerListener({required this.child});

  @override
  State<_PointerListener> createState() => _PointerListenerState();
}

class _PointerListenerState extends State<_PointerListener> {
  @override
  Widget build(BuildContext context) {
    bool isClosing = context.select((WindowState state) => state.isClosing);

    return IgnorePointer(
      ignoring: isClosing,
      child: Listener(
        onPointerDown: (_) {
          var state = context.read<WindowState>();
          PlatformApi.activateWindow(state.viewId);
        },
        child: widget.child,
      ),
    );
  }
}

class _Surface extends StatelessWidget {
  const _Surface({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var state = context.read<WindowState>();

    return Selector(
      selector: (_, WindowState state) => state.surfaceSize,
      builder: (_, Size size, Widget? child) {
        return SizedBox(
          width: size.width,
          height: size.height,
          child: child,
        );
      },
      child: Stack(
        clipBehavior: Clip.none,
        children: [
          ViewInputListener(
            viewId: state.viewId,
            child: Texture(
              key: state.textureKey,
              filterQuality: FilterQuality.medium,
              textureId: state.viewId,
            ),
          ),
          Selector(
            selector: (_, WindowState windowState) => windowState.popups,
            builder: (_, List<Popup> popups, __) {
              return Stack(
                clipBehavior: Clip.none,
                children: popups,
              );
            },
          ),
        ],
      ),
    );
  }
}
