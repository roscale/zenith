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

static void server_new_pointer(struct flutland_server* server,
                               struct wlr_input_device* device) {
	/* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another compositor, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */
	wlr_cursor_attach_input_device(server->cursor, device);
}

void server_new_input(struct wl_listener* listener, void* data) {
	/* This event is raised by the backend when a new input device becomes
	 * available. */
	struct flutland_server* server =
		  wl_container_of(listener, server, new_input);
	struct wlr_input_device* device = static_cast<struct wlr_input_device*>(data);
	switch (device->type) {
//		case WLR_INPUT_DEVICE_KEYBOARD:
//			server_new_keyboard(server, device);
//			break;
		case WLR_INPUT_DEVICE_POINTER: {
			std::clog << "new pointer" << std::endl;
			server_new_pointer(server, device);

//			FlutterPointerEvent e = {
//				  .struct_size = sizeof(FlutterPointerEvent),
//				  .phase = kAdd,
//				  .timestamp = FlutterEngineGetCurrentTime(),
//				  .x = event->x,
//				  .y = event->y,
//				  .signal_kind = kFlutterPointerSignalKindNone,
//				  .device_kind = kFlutterPointerDeviceKindMouse,
//			};
//			FlutterEngineSendPointerEvent(server->output->engine, &e, 1);

			break;
		}
		default:
			break;
	}
	/* We need to let the wlr_seat know what our capabilities are, which is
	 * communicated to the client. In TinyWL we always have a cursor, even if
	 * there are no pointer devices, so we always include that capability. */
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
//	if (!wl_list_empty(&server->keyboards)) {
//		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
//	}
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

	std::clog << "Cursor pos " << event->x << event->y << std::endl;

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
	wlr_seat_pointer_notify_button(server->seat,
	                               event->time_msec, event->button, event->state);

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