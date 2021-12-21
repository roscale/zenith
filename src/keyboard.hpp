#pragma once

#include "keyboard.hpp"
#include <wayland-util.h>

extern "C" {
#define static
#include <wlr/types/wlr_input_device.h>
#undef static
}

struct ZenithServer;

struct ZenithKeyboard {
	ZenithKeyboard(ZenithServer* server, wlr_input_device* device);

	ZenithServer* server;
	wlr_input_device* device;

	wl_listener modifiers{};
	wl_listener key{};
};

/*
 * This event is raised when a modifier key, such as shift or alt, is
 * pressed. We simply communicate this to the client.
 */
void keyboard_handle_modifiers(wl_listener* listener, void* data);

/*
 * This event is raised when a key is pressed or released.
 */
void keyboard_handle_key(wl_listener* listener, void* data);
