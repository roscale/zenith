import 'package:flutter/material.dart';
import 'package:zenith/widgets/desktop.dart';

void main() {
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
