#include "pointer.hpp"
#include "server.hpp"

extern "C" {
#define static
#include <wlr/types/wlr_pointer.h>
#undef static
}

ZenithPointer::ZenithPointer(ZenithServer* server)
	  : server(server) {
	/*
	 * Creates a cursor, which is a wlroots utility for tracking the cursor
	 * image shown on screen.
	 */
	cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(cursor, server->output_layout);

	/* Creates an xcursor manager, another wlroots utility which loads up
     * Xcursor themes to source cursor images from and makes sure that cursor
     * images are available at all scale factors on the screen (necessary for
     * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
	cursor_mgr = wlr_xcursor_manager_create(nullptr, 20);
	wlr_xcursor_manager_load(cursor_mgr, 1);

	/*
	 * wlr_cursor *only* displays an image on screen. It does not move around
	 * when the pointer moves. However, we can attach input devices to it, and
	 * it will generate aggregate events for all of them. In these events, we
	 * can choose how we want to process them, forwarding them to clients and
	 * moving the cursor around. More detail on this process is described in my
	 * input handling blog post:
	 *
	 * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
	 *
	 * And more comments are sprinkled throughout the notify functions above.
	 */
	cursor_motion.notify = server_cursor_motion;
	wl_signal_add(&cursor->events.motion, &cursor_motion);

	cursor_motion_absolute.notify = server_cursor_motion_absolute;
	wl_signal_add(&cursor->events.motion_absolute, &cursor_motion_absolute);

	cursor_button.notify = server_cursor_button;
	wl_signal_add(&cursor->events.button, &cursor_button);

	cursor_axis.notify = server_cursor_axis;
	wl_signal_add(&cursor->events.axis, &cursor_axis);

	cursor_frame.notify = server_cursor_frame;
	wl_signal_add(&cursor->events.frame, &cursor_frame);
}

void server_cursor_motion(wl_listener* listener, void* data) {
	ZenithPointer* pointer = wl_container_of(listener, pointer, cursor_motion);
	ZenithServer* server = pointer->server;
	auto* event = static_cast<wlr_event_pointer_motion*>(data);
	pointer->kinetic_scrolling = false;

	/* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */
	wlr_cursor_move(pointer->cursor, event->device, event->delta_x, event->delta_y);

	FlutterPointerEvent e = {};
	e.struct_size = sizeof(FlutterPointerEvent);
	e.phase = pointer->mouse_button_tracker.are_any_buttons_pressed() ? kMove : kHover;
	e.timestamp = FlutterEngineGetCurrentTime();
	e.x = pointer->cursor->x;
	e.y = pointer->cursor->y;
	e.device_kind = kFlutterPointerDeviceKindMouse;
	e.buttons = pointer->mouse_button_tracker.get_flutter_mouse_state();

	FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
}

void server_cursor_motion_absolute(wl_listener* listener, void* data) {
	ZenithPointer* pointer = wl_container_of(listener, pointer, cursor_motion_absolute);
	ZenithServer* server = pointer->server;
	auto* event = static_cast<wlr_event_pointer_motion_absolute*>(data);
	pointer->kinetic_scrolling = false;

	wlr_cursor_warp_absolute(pointer->cursor, event->device, event->x, event->y);

	FlutterPointerEvent e = {};
	e.struct_size = sizeof(FlutterPointerEvent);
	e.phase = pointer->mouse_button_tracker.are_any_buttons_pressed() ? kMove : kHover;
	e.timestamp = FlutterEngineGetCurrentTime();
	// Map from [0, 1] to [screen_width, screen_height].
	e.x = event->x * server->output->wlr_output->width;
	e.y = event->y * server->output->wlr_output->height;
	e.device_kind = kFlutterPointerDeviceKindMouse;
	e.buttons = pointer->mouse_button_tracker.get_flutter_mouse_state();

	FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
}

void server_cursor_button(wl_listener* listener, void* data) {
	ZenithPointer* pointer = wl_container_of(listener, pointer, cursor_button);
	ZenithServer* server = pointer->server;
	auto* event = static_cast<wlr_event_pointer_button*>(data);

	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);

	if (event->state == WLR_BUTTON_RELEASED) {
		pointer->mouse_button_tracker.release_button(event->button);

		FlutterPointerEvent e = {};
		e.struct_size = sizeof(FlutterPointerEvent);
		e.phase = pointer->mouse_button_tracker.are_any_buttons_pressed() ? kMove : kUp;
		e.timestamp = FlutterEngineGetCurrentTime();
		e.x = pointer->cursor->x;
		e.y = pointer->cursor->y;
		e.device_kind = kFlutterPointerDeviceKindMouse;
		e.buttons = pointer->mouse_button_tracker.get_flutter_mouse_state();

		FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
	} else {
		bool are_any_buttons_pressed = pointer->mouse_button_tracker.are_any_buttons_pressed();
		pointer->mouse_button_tracker.press_button(event->button);

		FlutterPointerEvent e = {};
		e.struct_size = sizeof(FlutterPointerEvent);
		e.phase = are_any_buttons_pressed ? kMove : kDown;
		e.timestamp = FlutterEngineGetCurrentTime();
		e.x = pointer->cursor->x;
		e.y = pointer->cursor->y;
		e.device_kind = kFlutterPointerDeviceKindMouse;
		e.buttons = pointer->mouse_button_tracker.get_flutter_mouse_state();

		FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
	}
}

void server_cursor_axis(wl_listener* listener, void* data) {
	ZenithPointer* pointer = wl_container_of(listener, pointer, cursor_axis);
	ZenithServer* server = pointer->server;
	auto* event = static_cast<wlr_event_pointer_axis*>(data);

	uint32_t now = FlutterEngineGetCurrentTime() / 1'000'000;

	uint32_t interval = 30;

	std::cout << "event->delta = " << event->delta << std::endl;
	std::cout << "event->delta_discrete = " << event->delta_discrete << std::endl;
	std::cout << "event->source = " << event->source << std::endl;
	std::cout << "event->time_msec = " << event->time_msec << std::endl;
	std::cout << "event->orientation = " << event->orientation << std::endl;
	std::cout << "event->device = " << event->device << std::endl;
	std::cout << std::endl;

	if (event->source == WLR_AXIS_SOURCE_FINGER) {
		if (abs(event->delta) < 0.1 && abs(pointer->kinetic_event.delta) < 0.1) {
			std::cout << "huh" << std::endl;
			return;
		}

		if (abs(event->delta) >= 0.1) {
			pointer->kinetic_scrolling = false;
			pointer->kinetic_events.push_back(*event);

			uint32_t oldest_threshold = now - interval;
			while (not pointer->kinetic_events.empty()) {
				auto& front = pointer->kinetic_events.front();
				if (front.time_msec < oldest_threshold) {
					pointer->kinetic_events.pop_front();
				} else {
					break;
				}
			}
			pointer->kinetic_event = *event;

			std::cout << "ms " << event->time_msec << std::endl;

		} else {
//			uint32_t oldest_threshold = now - interval;
//			while (not pointer->kinetic_events.empty()) {
//				auto& front = pointer->kinetic_events.front();
//				if (front.time_msec < oldest_threshold) {
//					pointer->kinetic_events.pop_front();
//				} else {
//					break;
//				}
//			}

			if (not pointer->kinetic_events.empty()) {
				double sum = 0;
				for (auto& kevent: pointer->kinetic_events) {
					sum += kevent.delta;
				}
				pointer->mean_delta = sum / pointer->kinetic_events.size();
			} else {
				pointer->mean_delta = 0;
				std::cout << "mean delta = 0" << std::endl;
			}

			if (abs(pointer->mean_delta) >= 0.1 && abs(pointer->kinetic_events.back().delta) > 0.5) {
				pointer->kinetic_scrolling = true;
				pointer->last_kinetic_event = now;
				pointer->last_real_event = event->time_msec;
				std::cout << "Start scroll" << std::endl;
				wlr_seat_pointer_notify_axis(server->seat,
				                             now, event->orientation,
				                             pointer->mean_delta,
				                             0, event->source);
				wlr_seat_pointer_notify_frame(server->seat);
			}
		}
	}

	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(server->seat,
	                             event->time_msec, event->orientation, event->delta,
	                             event->delta_discrete, event->source);

	bool are_any_buttons_pressed = pointer->mouse_button_tracker.are_any_buttons_pressed();

	FlutterPointerEvent e = {};
	e.struct_size = sizeof(FlutterPointerEvent);
	e.phase = are_any_buttons_pressed ? kMove : kDown;
	e.timestamp = FlutterEngineGetCurrentTime();
	e.x = pointer->cursor->x;
	e.y = pointer->cursor->y;
	e.device_kind = kFlutterPointerDeviceKindMouse;
	e.buttons = pointer->mouse_button_tracker.get_flutter_mouse_state();
	e.signal_kind = kFlutterPointerSignalKindScroll;
	e.scroll_delta_y = event->delta;

	FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
}

void server_cursor_frame(wl_listener* listener, void* data) {
	ZenithPointer* pointer = wl_container_of(listener, pointer, cursor_frame);
	ZenithServer* server = pointer->server;

	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(server->seat);
}
