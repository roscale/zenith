import 'package:flutter/material.dart';
import 'package:zenith/widgets/desktop.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  WidgetsBinding.instance.addPersistentFrameCallback((timeStamp) {
    // FIXME: FlutterEngineMarkExternalTextureFrameAvailable does not trigger a VSync fast enough,
    // so Flutter will only VSync every second frame. Marking the texture after FlutterEngineOnVsync
    // only fixes the problem partially because Flutter will still skip frames every once in a while.
    // This forces Flutter to always schedule a new frame.
    WidgetsBinding.instance.scheduleFrame();
  });
  runApp(const Zenith());
}

class Zenith extends StatelessWidget {
  const Zenith({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Zenith',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const Scaffold(
        body: Desktop(),
      ),
    );
  }
}
