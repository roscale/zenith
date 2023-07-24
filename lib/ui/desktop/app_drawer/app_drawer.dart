import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:zenith/ui/common/state/app_drawer.dart';
import 'package:zenith/ui/desktop/app_drawer/app_grid.dart';

class AppDrawer extends StatelessWidget {
  const AppDrawer({super.key});

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 600,
      height: 600,
      child: ClipRRect(
        borderRadius: BorderRadius.circular(10),
        child: BackdropFilter(
          filter: ImageFilter.blur(
            sigmaX: 10,
            sigmaY: 10,
          ),
          child: Container(
            color: Colors.white54,
            child: Column(
              children: [
                Padding(
                  padding: const EdgeInsets.fromLTRB(30, 30, 30, 10),
                  child: _AppDrawerTextField(),
                ),
                const Expanded(
                  child: AppGrid(),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class _AppDrawerTextField extends ConsumerStatefulWidget {
  @override
  ConsumerState<_AppDrawerTextField> createState() => _AppDrawerTextFieldState();
}

class _AppDrawerTextFieldState extends ConsumerState<_AppDrawerTextField> {
  final _searchController = TextEditingController();
  final _searchFocusNode = FocusNode();

  @override
  void initState() {
    super.initState();
    _searchController.addListener(() {
      ref.read(appDrawerFilterProvider.notifier).state = _searchController.text;
    });
    _searchFocusNode.requestFocus();
  }

  @override
  Widget build(BuildContext context) {
    return TextField(
      controller: _searchController,
      focusNode: _searchFocusNode,
      autofocus: true,
      decoration: InputDecoration(
        prefixIcon: const Padding(
          padding: EdgeInsets.fromLTRB(15, 0, 10, 0),
          child: Icon(Icons.search),
        ),
        prefixIconColor: Colors.grey,
        hintText: "Search apps",
        hintStyle: const TextStyle(color: Colors.grey),
        filled: true,
        fillColor: Colors.white,
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(5),
          borderSide: BorderSide.none,
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(5),
          borderSide: BorderSide.none,
        ),
      ),
    );
  }

  @override
  void dispose() {
    _searchController.dispose();
    _searchFocusNode.dispose();
    super.dispose();
  }
}
