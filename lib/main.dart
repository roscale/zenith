import 'package:elinux_app/desktop_state.dart';
import 'package:elinux_app/window.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: ChangeNotifierProvider(
        create: (_) => DesktopState(),
        child: const Desktop(),
      ),
    );
  }
}

class Desktop extends StatefulWidget {
  const Desktop({Key? key}) : super(key: key);

  @override
  _DesktopState createState() => _DesktopState();
}

class _DesktopState extends State<Desktop> {
  @override
  Widget build(BuildContext context) {
    var desktopState = context.watch<DesktopState>();

    return Container(
      color: Colors.white,
      child: Stack(
        children: [...desktopState.windows, const CircularProgressIndicator()],
      ),
    );
  }
}
