#pragma once

#include <wayland-server-core.h>
#include "message_structs.hpp"

/*
 * Wayland expects clients to implement the key repeat functionality but the Flutter engine has
 * to receive these events if we want key repeat to work in text fields. This class synthesizes
 * key repeat events by scheduling callbacks on the main event loop.
 */
class KeyRepeater {
	KeyboardKeyEventMessage repeated_event{};
	int32_t delay{};
	int32_t rate{};

	wl_event_source* timer = nullptr;

public:

	void schedule_repeat(KeyboardKeyEventMessage repeated_event);

	void cancel_repeat();
};
