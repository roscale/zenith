#include "mouse_button_tracker.hpp"
#include <linux/input-event-codes.h>
#include <iostream>
#include "embedder.h"

void MouseButtonTracker::press_button(uint32_t button) {
	pressed_buttons.insert(button);
}

void MouseButtonTracker::release_button(uint32_t button) {
	pressed_buttons.erase(button);
}

bool MouseButtonTracker::is_button_pressed(uint32_t button) {
	return pressed_buttons.find(button) != pressed_buttons.end();
}

bool MouseButtonTracker::are_any_buttons_pressed() {
	return !pressed_buttons.empty();
}

int64_t MouseButtonTracker::get_flutter_mouse_state() {
	int64_t bitmap = 0;
	for (uint32_t button: pressed_buttons) {
		switch (button) {
			case BTN_LEFT:
				bitmap |= kFlutterPointerButtonMousePrimary;
				break;
			case BTN_RIGHT:
				bitmap |= kFlutterPointerButtonMouseSecondary;
				break;
			case BTN_MIDDLE:
				bitmap |= kFlutterPointerButtonMouseMiddle;
				break;
			case BTN_BACK:
				bitmap |= kFlutterPointerButtonMouseBack;
				break;
			case BTN_FORWARD:
				bitmap |= kFlutterPointerButtonMouseForward;
				break;
			default:
				std::cerr << "Unrecognized mouse button " << button << std::endl;
		}
	}
	return bitmap;
}