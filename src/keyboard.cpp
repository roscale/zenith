#include "keyboard.hpp"
#include <wayland-util.h>
#include <malloc.h>
#include "server.hpp"

extern "C" {
#define static
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/gles2.h>
#undef static
}

ZenithKeyboard::ZenithKeyboard(ZenithServer* server, wlr_input_device* device)
	  : server(server), device(device) {
	/* We need to prepare an XKB keymap and assign it to the keyboard. This
	 * assumes the defaults (e.g. layout = "us"). */
	xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	xkb_keymap* keymap = xkb_keymap_new_from_names(context, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(device->keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(device->keyboard, 25, 300);

	/* Here we set up listeners for keyboard events. */
	modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&device->keyboard->events.modifiers, &modifiers);
	key.notify = keyboard_handle_key;
	wl_signal_add(&device->keyboard->events.key, &key);

	wlr_seat_set_keyboard(server->seat, device);
}

void keyboard_handle_modifiers(wl_listener* listener, void* data) {
	ZenithKeyboard* keyboard = wl_container_of(listener, keyboard, modifiers);
	/*
	 * A seat can only have one keyboard, but this is a limitation of the
	 * Wayland protocol - not wlroots. We assign all connected keyboards to the
	 * same seat. You can swap out the underlying wlr_keyboard like this and
	 * wlr_seat handles this transparently.
	 */
	wlr_seat_set_keyboard(keyboard->server->seat, keyboard->device);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(keyboard->server->seat, &keyboard->device->keyboard->modifiers);
}

void keyboard_handle_key(wl_listener* listener, void* data) {
	ZenithKeyboard* keyboard = wl_container_of(listener, keyboard, key);
	ZenithServer* server = keyboard->server;

	auto* event = static_cast<wlr_event_keyboard_key*>(data);
	wlr_seat* seat = server->seat;

	// We pass it along to the client.
	wlr_seat_set_keyboard(seat, keyboard->device);
	wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
}