#include "pointer.hpp"
#include "server.hpp"
#include "defer.hpp"
#include <cmath>

extern "C" {
#define static
#include "wlr/types/wlr_pointer.h"
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

	kinetic_scrolling_timer = wl_event_loop_add_timer(wl_display_get_event_loop(server->display),
	                                                  kinetic_scrolling_timer_callback, this);
	// Arm the timer.
	wl_event_source_timer_update(kinetic_scrolling_timer, 1);
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

	auto f = [&event, &pointer]() {
		const double epsilon = 0.0001;
		uint32_t now = FlutterEngineGetCurrentTime() / 1'000'000;

		if (event->source != WLR_AXIS_SOURCE_FINGER) {
			return;
		}

		if (abs(event->delta) >= epsilon) {
			// Real movement, not a stop event.
			pointer->kinetic_scrolling = false;
			pointer->recent_scroll_events.push_back(*event);
			// Keep scroll events only from the last 30 ms.
			const uint32_t interval = 30;
			uint32_t oldest_threshold = now - interval;
			while (not pointer->recent_scroll_events.empty()) {
				auto& front = pointer->recent_scroll_events.front();
				if (front.time_msec < oldest_threshold) {
					pointer->recent_scroll_events.pop_front();
				} else {
					break;
				}
			}
			// Save the latest one.
			pointer->last_real_scroll_event = *event;
			return;
		}

		// Stop event.

		defer _([&]() {
			// Clear the scroll events on return.
			pointer->recent_scroll_events.clear();
		});

		if (pointer->recent_scroll_events.empty()) {
			// No scroll events, nothing to do.
			return;
		}

		double sum_delta_x = 0.0;
		double sum_delta_y = 0.0;
		size_t num_delta_x = 0;
		size_t num_delta_y = 0;

		wlr_event_pointer_axis* last_horizontal_event = nullptr;
		wlr_event_pointer_axis* last_vertical_event = nullptr;

		for (auto& e: pointer->recent_scroll_events) {
			if (e.orientation == WLR_AXIS_ORIENTATION_HORIZONTAL) {
				last_horizontal_event = &e;
				sum_delta_x += e.delta;
				num_delta_x += 1;
			} else if (e.orientation == WLR_AXIS_ORIENTATION_VERTICAL) {
				last_vertical_event = &e;
				sum_delta_y += e.delta;
				num_delta_y += 1;
			}
		}

		double average_delta_x = num_delta_x != 0 ? sum_delta_x / (double) num_delta_x : 0.0;
		double average_delta_y = num_delta_y != 0 ? sum_delta_y / (double) num_delta_y : 0.0;
		// Pythagorean theorem.
		double average_delta = average_delta_x * average_delta_x + average_delta_y * average_delta_y;

		if (average_delta < 1.0 * 1.0) { // TODO: something other than epsilon
			// Insignificant scroll events.
			return;
		}

		if ((last_horizontal_event == nullptr || abs(last_horizontal_event->delta) < 1.0) &&
		    (last_vertical_event == nullptr || abs(last_vertical_event->delta) < 1.0)) {
			// The user most likely stopped scrolling to rest his fingers on the touchpad.
			return;
		}

		pointer->kinetic_scrolling = true;
		pointer->average_delta_x = average_delta_x;
		pointer->average_delta_y = average_delta_y;
		pointer->last_kinetic_event_time = now;
		pointer->last_real_event = event->time_msec;
	};
	f();


//	if (event->source == WLR_AXIS_SOURCE_FINGER) {
////		if (abs(event->delta) <= epsilon && abs(pointer->kinetic_event.delta) <= epsilon) {
////			std::cout << "huh" << std::endl;
////			return;
////		}
//
//		if (abs(event->delta) > epsilon) {
//			// Real movement, not a stop event.
//			pointer->kinetic_scrolling = false;
//			pointer->recent_scroll_events.push_back(*event);
//
//			// Keep scroll events only from the last 30 ms.
//			const uint32_t interval = 30;
//			uint32_t oldest_threshold = now - interval;
//			while (not pointer->recent_scroll_events.empty()) {
//				auto& front = pointer->recent_scroll_events.front();
//				if (front.time_msec < oldest_threshold) {
//					pointer->recent_scroll_events.pop_front();
//				} else {
//					break;
//				}
//			}
//
//			// Save the latest one.
//			pointer->last_real_scroll_event = *event;
//			// return;
//		} else {
//			// Stop event.
//			if (not pointer->recent_scroll_events.empty()) {
//				double sum = 0;
//				for (auto& kevent: pointer->recent_scroll_events) {
//					sum += kevent.delta;
//				}
//				pointer->average_delta = sum / pointer->recent_scroll_events.size();
//			} else {
//				pointer->average_delta = 0.0;
//				std::cout << "mean delta = 0" << std::endl;
//			}
//
//			if (abs(pointer->average_delta) >= epsilon && abs(pointer->recent_scroll_events.back().delta) > 0.5) {
//				pointer->kinetic_scrolling = true;
//				pointer->last_kinetic_event = now;
//				pointer->last_real_event = event->time_msec;
//				std::cout << "Start scroll" << std::endl;
//				wlr_seat_pointer_notify_axis(server->seat,
//				                             now, event->orientation,
//				                             pointer->average_delta,
//				                             0, event->source);
//				wlr_seat_pointer_notify_frame(server->seat);
//			}
//		}
//	}

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

int kinetic_scrolling_timer_callback(void* data) {
	auto* pointer = static_cast<ZenithPointer*>(data);
//	ZenithServer* server = pointer->server;



	wl_event_source_timer_update(pointer->kinetic_scrolling_timer, 1);
	return 0;
}