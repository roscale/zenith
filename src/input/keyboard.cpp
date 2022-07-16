#include "keyboard.hpp"
#include "server.hpp"

extern "C" {
#include <wayland-util.h>
#include <linux/vt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
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

	bool shortcut_handled = false;
	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);

		/* Translate libinput keycode -> xkbcommon */
		uint32_t keycode = event->keycode + 8;
		/* Get a list of keysyms based on the keymap for this keyboard */
		const xkb_keysym_t* syms;
		int nsyms = xkb_state_key_get_syms(keyboard->device->keyboard->xkb_state, keycode, &syms);

		shortcut_handled = handle_shortcuts(keyboard, modifiers, syms, (size_t) nsyms);
	}

	if (not shortcut_handled) {
		// We pass it along to the client.
		wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
	}
}

bool handle_shortcuts(struct ZenithKeyboard* keyboard, uint32_t modifiers, const xkb_keysym_t* syms, size_t nsyms) {
	if (modifiers == 0) {
		// No need to check shortcuts if no modifier is pressed.
		return false;
	}

	std::unordered_set<xkb_keysym_t> sym_set{};

	for (size_t i = 0; i < nsyms; i++) {
		sym_set.insert(syms[i]);
	}

	auto is_key_pressed = [&sym_set](uint64_t key) {
		return sym_set.find(key) != sym_set.end();
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
