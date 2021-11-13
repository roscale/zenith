#include "input_callbacks.hpp"
#include "flutland_structs.hpp"
#include "mouse_button_tracker.hpp"
#include <bitset>

extern "C" {
#define static
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

#include <semaphore.h>
#include <malloc.h>
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#undef static
}

MouseButtonTracker mouse_button_tracker;

void process_cursor_motion(struct flutland_server* server, uint32_t time) {
	/* If the mode is non-passthrough, delegate to those functions. */
//	if (server->cursor_mode == TINYWL_CURSOR_MOVE) {
////		process_cursor_move(server, time);
//		return;
//	} else if (server->cursor_mode == TINYWL_CURSOR_RESIZE) {
////		process_cursor_resize(server, time);
//		return;
//	}

	/* Otherwise, find the view under the pointer and send the event along. */
	double sx, sy;
	struct wlr_seat* seat = server->seat;
	struct wlr_surface* surface = NULL;
//	struct flutland_view *view = desktop_view_at(server,
//	                                           server->cursor->x, server->cursor->y, &surface, &sx, &sy);
//	if (!view) {
	/* If there's no view under the cursor, set the cursor image to a
	 * default. This is what makes the cursor image appear when you move it
	 * around the screen, not over any views. */
//	FlutterEnginePostRenderThreadTask(server->output->engine, [](void* userdata){
//		auto* server = static_cast<struct flutland_server*>(userdata);
//		wlr_xcursor_manager_set_cursor_image(
//			  server->cursor_mgr, "left_ptr", server->cursor);
//	}, server);
//	wlr_egl_context saved;
//	wlr_egl_save_context(&saved);
//	wlr_egl_restore_context(server->output->platform_thread_egl_context)


//	glBindTexture(GL_TEXTURE_2D, 0);



//	}
//	if (surface) {
	/*
	 * Send pointer enter and motion events.
	 *
	 * The enter event gives the surface "pointer focus", which is distinct
	 * from keyboard focus. You get pointer focus by moving the pointer over
	 * a window.
	 *
	 * Note that wlroots will avoid sending duplicate enter/motion events if
	 * the surface has already has pointer focus or if the client is already
	 * aware of the coordinates passed.
	 */
//		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
//		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
//	} else {
//		/* Clear pointer focus so future button events and such are not sent to
//		 * the last client to have the cursor over it. */
//		wlr_seat_pointer_clear_focus(seat);
//	}
}

static void server_new_pointer(struct flutland_server* server, struct wlr_input_device* device) {
	/* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another compositor, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */
	wlr_cursor_attach_input_device(server->cursor, device);
}

static void server_new_keyboard(struct flutland_server* server, struct wlr_input_device* device) {
	struct flutland_keyboard* keyboard = static_cast<flutland_keyboard*>(calloc(1, sizeof(struct flutland_keyboard)));
	keyboard->server = server;
	keyboard->device = device;

	/* We need to prepare an XKB keymap and assign it to the keyboard. This
	 * assumes the defaults (e.g. layout = "us"). */
	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, NULL,
	                                                      XKB_KEYMAP_COMPILE_NO_FLAGS);

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

void server_new_input(struct wl_listener* listener, void* data) {
	/* This event is raised by the backend when a new input device becomes
	 * available. */
	struct flutland_server* server =
			wl_container_of(listener, server, new_input);
	struct wlr_input_device* device = static_cast<struct wlr_input_device*>(data);
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

void server_cursor_motion(struct wl_listener* listener, void* data) {
	/* This event is forwarded by the cursor when a pointer emits a _relative_
	 * pointer motion event (i.e. a delta) */
	struct flutland_server* server =
			wl_container_of(listener, server, cursor_motion);
	struct wlr_event_pointer_motion* event = static_cast<struct wlr_event_pointer_motion*>(data);
	/* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */
	wlr_cursor_move(server->cursor, event->device,
	                event->delta_x, event->delta_y);

	std::clog << "Motion: " << event->delta_x << " " << event->delta_y << "\n";

	process_cursor_motion(server, event->time_msec);

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

void server_cursor_motion_absolute(
		struct wl_listener* listener, void* data) {
//	std::clog << "Absolute motion" << "\n";

	/* This event is forwarded by the cursor when a pointer emits an _absolute_
	 * motion event, from 0..1 on each axis. This happens, for example, when
	 * wlroots is running under a Wayland window rather than KMS+DRM, and you
	 * move the mouse over the window. You could enter the window from any edge,
	 * so we have to warp the mouse there. There is also some hardware which
	 * emits these events. */
	struct flutland_server* server =
			wl_container_of(listener, server, cursor_motion_absolute);
	struct wlr_event_pointer_motion_absolute* event = static_cast<struct wlr_event_pointer_motion_absolute*>(data);
	wlr_cursor_warp_absolute(server->cursor, event->device, event->x, event->y);
	process_cursor_motion(server, event->time_msec);

//	std::clog << "Cursor pos " << event->x << event->y << std::endl;

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

void server_cursor_button(struct wl_listener* listener, void* data) {
	/* This event is forwarded by the cursor when a pointer emits a button
	 * event. */
	struct flutland_server* server =
			wl_container_of(listener, server, cursor_button);
	struct wlr_event_pointer_button* event = static_cast<struct wlr_event_pointer_button*>(data);
	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);

	std::clog << "Button: " << std::bitset<32>(event->button) << "\n";

	double sx, sy;
	struct wlr_surface* surface;
//	struct flutland_view* view = desktop_view_at(server,
//	                                             server->cursor->x, server->cursor->y, &surface, &sx, &sy);
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
		std::clog << "Release" << std::endl;
		/* If you released any buttons, we exit interactive move/resize mode. */
//		server->cursor_mode = TINYWL_CURSOR_PASSTHROUGH;
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
		/* Focus that client if the button was _pressed_ */
//		focus_view(view, surface);

		std::clog << "Press " << server->cursor->x << std::endl;
	}
}

void server_cursor_axis(struct wl_listener* listener, void* data) {
	std::clog << "Axis" << "\n";

	/* This event is forwarded by the cursor when a pointer emits an axis event,
	 * for example when you move the scroll wheel. */
	struct flutland_server* server = wl_container_of(listener, server, cursor_axis);
	struct wlr_event_pointer_axis* event = static_cast<struct wlr_event_pointer_axis*>(data);
	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(server->seat,
	                             event->time_msec, event->orientation, event->delta,
	                             event->delta_discrete, event->source);
}

void server_cursor_frame(struct wl_listener* listener, void* data) {
	/* This event is forwarded by the cursor when a pointer emits a frame
	 * event. Frame events are sent after regular pointer events to group
	 * multiple events together. For instance, two axis events may happen at the
	 * same time, in which case a frame event won't be sent in between. */
	struct flutland_server* server = wl_container_of(listener, server, cursor_frame);
	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(server->seat);
}

void keyboard_handle_modifiers(struct wl_listener* listener, void* data) {
	/* This event is raised when a modifier key, such as shift or alt, is
	 * pressed. We simply communicate this to the client. */
	struct flutland_keyboard* keyboard = wl_container_of(listener, keyboard, modifiers);
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

void keyboard_handle_key(struct wl_listener* listener, void* data) {
	/* This event is raised when a key is pressed or released. */
	struct flutland_keyboard* keyboard = wl_container_of(listener, keyboard, key);
	struct flutland_server* server = keyboard->server;
	auto* event = static_cast<struct wlr_event_keyboard_key*>(data);
	struct wlr_seat* seat = server->seat;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t* syms;
	int nsyms = xkb_state_key_get_syms(keyboard->device->keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);
//	if ((modifiers & WLR_MODIFIER_ALT) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
	/* If alt is held down and this button was _pressed_, we attempt to
	 * process it as a compositor keybinding. */
//		for (int i = 0; i < nsyms; i++) {
//			handled = handle_keybinding(server, syms[i]);
//		}
//	}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(seat, keyboard->device);
		wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
	}
}

void focus_view(struct flutland_view* view) {
	/* Note: this function only deals with keyboard focus. */
	if (view == nullptr) {
		return;
	}

	struct flutland_server* server = view->server;
	struct wlr_seat* seat = server->seat;
	struct wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

	if (view->xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		return;
	}

	struct wlr_surface* prev_surface = seat->keyboard_state.focused_surface;
	if (prev_surface == view->xdg_surface->surface) {
		/* Don't re-focus an already focused surface. */
		return;
	}
	if (prev_surface) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		struct wlr_xdg_surface* previous = wlr_xdg_surface_from_wlr_surface(
				seat->keyboard_state.focused_surface);
		wlr_xdg_toplevel_set_activated(previous, false);
	}
	/* Move the view to the front */
	server->views.erase(server->views.find(view->xdg_surface->surface));
//	wl_list_remove(&view->link);
	server->views.insert(std::make_pair(view->xdg_surface->surface, view));
//	wl_list_insert(&server->views, &view->link);
	/* Activate the new surface */
	wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface,
	                               keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}