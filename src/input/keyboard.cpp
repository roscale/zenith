#include <libinput.h>
#include "keyboard.hpp"
#include "server.hpp"
#include "encodable_value.h"
#include "json_message_codec.h"

extern "C" {
#include <wayland-util.h>
#include <linux/vt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define static
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/types/wlr_xdg_shell.h>
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
	wlr_seat* seat = keyboard->server->seat;

	/*
	 * A seat can only have one keyboard, but this is a limitation of the
	 * Wayland protocol - not wlroots. We assign all connected keyboards to the
	 * same seat. You can swap out the underlying wlr_keyboard like this and
	 * wlr_seat handles this transparently.
	 */
	wlr_seat_set_keyboard(seat, keyboard->device);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(seat, &keyboard->device->keyboard->modifiers);
}

void keyboard_handle_key(wl_listener* listener, void* data) {
	ZenithKeyboard* keyboard = wl_container_of(listener, keyboard, key);
	wlr_seat* seat = keyboard->server->seat;
	auto* event = static_cast<wlr_event_keyboard_key*>(data);

	wlr_seat_set_keyboard(seat, keyboard->device);

	// Translate libinput keycode to xkbcommon.
	// This is actually a scan code because it's independent of the keyboard layout.
	// https://code.woboq.org/gtk/include/xkbcommon/xkbcommon.h.html#160
	xkb_keycode_t scan_code = event->keycode + 8;

	xkb_keysym_t keysym = xkb_state_key_get_one_sym(keyboard->device->keyboard->xkb_state, scan_code);

	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);

	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		bool shortcut_handled = handle_shortcuts(keyboard, modifiers, keysym);
		if (shortcut_handled) {
			return;
		}
	}

	auto message = KeyboardKeyEventMessage{
		  .event = *event,
		  .scan_code = scan_code,
		  .keysym = keysym,
		  .modifiers = modifiers,
	};
	ZenithServer::instance()->embedder_state->send_key_event(message);
}

bool handle_shortcuts(struct ZenithKeyboard* keyboard, uint32_t modifiers, xkb_keysym_t keysym) {
	auto is_key_pressed = [&keysym](xkb_keysym_t key) {
		return key == keysym;
	};

	auto is_modifier_pressed = [&modifiers](wlr_keyboard_modifier modifier) {
		return (modifiers & modifier) != 0;
	};

	// Alt + Esc
	if (is_modifier_pressed(WLR_MODIFIER_ALT) and is_key_pressed(XKB_KEY_Escape)) {
		wl_display_terminate(keyboard->server->display);
		return true;
	}

	// Ctrl + Alt + F<num>
	for (int vt = 1; vt <= 12; vt++) {
		if (is_key_pressed(XKB_KEY_XF86Switch_VT_1 + vt - 1)) {
			int fd = open("/dev/tty", O_RDWR);
			if (fd > 0) {
				ioctl(fd, VT_ACTIVATE, vt);
				ioctl(fd, VT_WAITACTIVE, vt);
				close(fd);
			}
			return true;
		}
	}

	return false;
}
