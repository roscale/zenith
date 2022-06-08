import 'package:get_it/get_it.dart';
import 'package:zenith/util/mouse_button_tracker.dart';
import 'package:zenith/util/pointer_focus_manager.dart';

final getIt = GetIt.instance;

void registerServices() {
  getIt.registerSingleton(PointerFocusManager());
  getIt.registerSingleton(MouseButtonTracker());
}
