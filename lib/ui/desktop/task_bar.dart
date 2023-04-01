import 'package:flutter/material.dart';

class TaskBar extends StatelessWidget {
  const TaskBar({super.key});

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      height: 50,
      child: Container(
        color: Colors.white38,
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            SizedBox(
              width: 100,
              child: TextField(
                keyboardType: TextInputType.multiline,
                maxLines: null,
                onSubmitted: (s) {
                  print("SUBMITTED $s");
                },
              ),
            ),
          ],
        ),
      ),
    );
  }
}
