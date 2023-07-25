import 'package:flutter/cupertino.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:riverpod_annotation/riverpod_annotation.dart';

part '../../generated/util/state/root_overlay.g.dart';

@Riverpod(keepAlive: true)
GlobalKey<OverlayState> rootOverlayKey(RootOverlayKeyRef ref) => GlobalKey();
