import 'dart:ui';

import 'package:flutter/material.dart';

class AppDrawer extends StatefulWidget {
  const AppDrawer({Key? key}) : super(key: key);

  @override
  State<AppDrawer> createState() => _AppDrawerState();
}

class _AppDrawerState extends State<AppDrawer> {
  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(0, 40, 0, 0),
      child: ClipRect(
        child: BackdropFilter(
          filter: ImageFilter.blur(sigmaX: 50, sigmaY: 50),
          child: Card(
            color: Colors.black26,
            shadowColor: Colors.transparent,
            margin: const EdgeInsets.all(0),
            child: Column(
              children: [

              ],
            ),
          ),
        ),
      ),
    );
  }
}
