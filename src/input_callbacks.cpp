#include "input_callbacks.hpp"
#include "zenith_structs.hpp"
#include "mouse_button_tracker.hpp"

extern "C" {
#include <semaphore.h>
#include <malloc.h>
#define static
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/types/wlr_cursor.h>
#undef static
}

MouseButtonTracker mouse_button_tracker;

/*
 * We don't do anything special with pointers. All of our pointer handling
 * is proxied through wlr_cursor. On another compositor, you might take this
 * opportunity to do libinput configuration on the device to set
 * acceleration, etc.
 */
static void server_new_pointer(ZenithServer* server, wlr_input_device* device) {
	wlr_cursor_attach_input_device(server->cursor, device);
}

static void server_new_keyboard(ZenithServer* server, wlr_input_device* device) {
	ZenithKeyboard* keyboard = static_cast<ZenithKeyboard*>(calloc(1, sizeof(ZenithKeyboard)));
	keyboard->server = server;
	keyboard->device = device;

	/* We need to prepare an XKB keymap and assign it to the keyboard. This
	 * assumes the defaults (e.g. layout = "us"). */
	xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	xkb_keymap* keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(device->keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(device->keyboard, 25, 300);

	/* Here we set up listeners for keyboard events. */
	keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);
	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&device->keyboard->events.key, &keyboard->key);

	wlr_seat_set_keyboard(server->seat, device);

	/* And add the keyboard to our list of keyboards */
	wl_list_insert(&server->keyboards, &keyboard->link);
}

void server_new_input(wl_listener* listener, void* data) {
	// TODO: Offset of on non-standard-layout type
	ZenithServer* server = wl_container_of(listener, server, new_input);
	auto* device = static_cast<wlr_input_device*>(data);

	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			server_new_keyboard(server, device);
			break;
		case WLR_INPUT_DEVICE_POINTER: {
			std::clog << "new pointer" << std::endl;
			server_new_pointer(server, device);
			break;
		}
		default:
			break;
	}
	/* We need to let the wlr_seat know what our capabilities are, which is
	 * communicated to the client. In TinyWL we always have a cursor, even if
	 * there are no pointer devices, so we always include that capability. */
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&server->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat, caps);
}

void server_cursor_motion(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, cursor_motion);
	auto* event = static_cast<wlr_event_pointer_motion*>(data);
	/* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */
	wlr_cursor_move(server->cursor, event->device, event->delta_x, event->delta_y);

	// TODO: Prevent pipeline stalling causing frame drops.
	wlr_xcursor_manager_set_cursor_image(server->cursor_mgr, "left_ptr", server->cursor);

	FlutterPointerEvent e = {
		  .struct_size = sizeof(FlutterPointerEvent),
		  .phase = mouse_button_tracker.are_any_buttons_pressed() ? kMove : kHover,
		  .timestamp = FlutterEngineGetCurrentTime(),
		  .x = server->cursor->x,
		  .y = server->cursor->y,
		  .device_kind = kFlutterPointerDeviceKindMouse,
		  .buttons = mouse_button_tracker.get_flutter_mouse_state(),
	};
	FlutterEngineSendPointerEvent(server->output->engine, &e, 1);
}

void server_cursor_motion_absolute(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, cursor_motion_absolute);
	auto* event = static_cast<wlr_event_pointer_motion_absolute*>(data);

	wlr_cursor_warp_absolute(server->cursor, event->device, event->x, event->y);

	// TODO: Prevent pipeline stalling causing frame drops.
	wlr_xcursor_manager_set_cursor_image(server->cursor_mgr, "left_ptr", server->cursor);

	FlutterPointerEvent e = {
		  .struct_size = sizeof(FlutterPointerEvent),
		  .phase = mouse_button_tracker.are_any_buttons_pressed() ? kMove : kHover,
		  .timestamp = FlutterEngineGetCurrentTime(),
		  .x = event->x * 1024,
		  .y = event->y * 768,
		  .device_kind = kFlutterPointerDeviceKindMouse,
		  .buttons = mouse_button_tracker.get_flutter_mouse_state(),
	};
	FlutterEngineSendPointerEvent(server->output->engine, &e, 1);
}

void server_cursor_button(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, cursor_button);
	auto* event = static_cast<wlr_event_pointer_button*>(data);

	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);

	if (event->state == WLR_BUTTON_RELEASED) {
		mouse_button_tracker.release_button(event->button);

		FlutterPointerEvent e = {
			  .struct_size = sizeof(FlutterPointerEvent),
			  .phase = mouse_button_tracker.are_any_buttons_pressed() ? kMove : kUp,
			  .timestamp = FlutterEngineGetCurrentTime(),
			  .x = server->cursor->x,
			  .y = server->cursor->y,
			  .device_kind = kFlutterPointerDeviceKindMouse,
			  .buttons = mouse_button_tracker.get_flutter_mouse_state(),
		};
		FlutterEngineSendPointerEvent(server->output->engine, &e, 1);
	} else {
		bool are_any_buttons_pressed = mouse_button_tracker.are_any_buttons_pressed();
		mouse_button_tracker.press_button(event->button);

		FlutterPointerEvent e = {
			  .struct_size = sizeof(FlutterPointerEvent),
			  .phase = are_any_buttons_pressed ? kMove : kDown,
			  .timestamp = FlutterEngineGetCurrentTime(),
			  .x = server->cursor->x,
			  .y = server->cursor->y,
			  .device_kind = kFlutterPointerDeviceKindMouse,
			  .buttons = mouse_button_tracker.get_flutter_mouse_state(),
		};
		FlutterEngineSendPointerEvent(server->output->engine, &e, 1);
	}
}

void server_cursor_axis(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, cursor_axis);
	auto* event = static_cast<wlr_event_pointer_axis*>(data);

	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(server->seat,
	                             event->time_msec, event->orientation, event->delta,
	                             event->delta_discrete, event->source);
}

void server_cursor_frame(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, cursor_frame);

	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(server->seat);
}

void server_seat_request_cursor(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, cursor_frame);

	auto* event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);
	wlr_seat_client* focused_client = server->seat->pointer_state.focused_client;
	/* This can be sent by any client, so we check to make sure this one is
	 * actually has pointer focus first. */
	if (focused_client == event->seat_client) {
		/* Once we've vetted the client, we can tell the cursor to use the
		 * provided surface as the cursor image. It will set the hardware cursor
		 * on the output that it's currently on and continue to do so as the
		 * cursor moves between outputs. */
		wlr_cursor_set_surface(server->cursor, event->surface, event->hotspot_x, event->hotspot_y);
	}
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

void focus_view(ZenithView* view) {
	if (view == nullptr || view->xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		// Popups cannot get focus.
		return;
	}

	wlr_seat* seat = view->server->seat;
	wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

	wlr_surface* prev_surface = seat->keyboard_state.focused_surface;

	bool is_surface_already_focused = prev_surface == view->xdg_surface->surface;
	if (is_surface_already_focused) {
		return;
	}

	if (prev_surface != nullptr) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		wlr_xdg_surface* previous = wlr_xdg_surface_from_wlr_surface(seat->keyboard_state.focused_surface);
		wlr_xdg_toplevel_set_activated(previous, false);
	}
	// Activate the new surface.
	wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface,
	                               keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}
