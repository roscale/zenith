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

	rapidjson::Document json;
	json.SetObject();
	json.AddMember("keymap", "linux", json.GetAllocator());
	// Even thought Flutter only understands GTK keycodes, these are essentially the same as
	// xkbcommon keycodes.
	// https://gitlab.gnome.org/GNOME/gtk/-/blob/gtk-3-24/gdk/wayland/gdkkeys-wayland.c#L179
	json.AddMember("toolkit", "gtk", json.GetAllocator());
	json.AddMember("scanCode", scan_code, json.GetAllocator());
	json.AddMember("keyCode", keysym, json.GetAllocator());
	// https://github.com/flutter/engine/blob/2a8ac1e0ca2535a6af17dde3530d277ecd601543/shell/platform/linux/fl_keyboard_manager.cc#L96
	if (keysym < 128) {
		json.AddMember("specifiedLogicalKey", keysym, json.GetAllocator());
	}
	json.AddMember("modifiers", modifiers, json.GetAllocator());
	// Normally I would also set `unicodeScalarValues`, but I don't anticipate using this feature.
	// https://github.com/flutter/flutter/blob/b8f7f1f986/packages/flutter/lib/src/services/raw_keyboard_linux.dart#L55

	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		json.AddMember("type", "keydown", json.GetAllocator());

		wlr_event_keyboard_key event_copy = *event;

		auto& keyEventChannel = keyboard->server->embedder_state->keyEventChannel;
		keyEventChannel.Send(json, [event_copy](const uint8_t* reply, size_t reply_size) {
			auto message = flutter::JsonMessageCodec::GetInstance().DecodeMessage(reply, reply_size);
			auto& handled_object = message->GetObject()["handled"];
			bool handled = handled_object.GetBool();

			// If the Flutter engine doesn't handle the key press, forward it to the Wayland client.
			if (!handled) {
				wlr_seat_keyboard_notify_key(ZenithServer::instance()->seat, event_copy.time_msec, event_copy.keycode,
				                             event_copy.state);
			}
		});
	} else if (event->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
		json.AddMember("type", "keyup", json.GetAllocator());

		wlr_event_keyboard_key event_copy = *event;

		auto& keyEventChannel = keyboard->server->embedder_state->keyEventChannel;
		keyEventChannel.Send(json, [event_copy](const uint8_t* reply, size_t reply_size) {
			auto message = flutter::JsonMessageCodec::GetInstance().DecodeMessage(reply, reply_size);
			auto& handled_object = message->GetObject()["handled"];
			bool handled = handled_object.GetBool();

			if (!handled) {
				wlr_seat_keyboard_notify_key(ZenithServer::instance()->seat, event_copy.time_msec, event_copy.keycode,
				                             event_copy.state);
			}
		});
	}
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
