#pragma once

#include <set>
#include <stdint.h>

class MouseButtonTracker {
	std::set<uint32_t> pressed_buttons;

public:
	void press_button(uint32_t button);

	void release_button(uint32_t button);

	bool is_button_pressed(uint32_t button);

	bool are_any_buttons_pressed();

	/// Returns a bitmap of pressed mouse buttons directly understood by the Flutter engine.
	int64_t get_flutter_mouse_state();
};