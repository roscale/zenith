#include "kinetic_scrolling.hpp"
#include "embedder.h"
#include "defer.hpp"
#include "time.hpp"

void KineticScrolling::record_scroll_event(wlr_event_pointer_axis* event) {
	const double epsilon = 0.0001;

	if (event->source != WLR_AXIS_SOURCE_FINGER) {
		return;
	}

	if (abs(event->delta) >= epsilon) {
		// Real movement, not a stop event. Only record the events while the user is manually scrolling.
		enabled = false;
		recent_scroll_events.push_back(*event);
		// Keep scroll events only from the last 30 ms.
		const uint32_t interval = 30;
		uint32_t oldest_threshold = current_time_milliseconds() - interval;
		while (not recent_scroll_events.empty()) {
			auto& front = recent_scroll_events.front();
			if (front.time_msec < oldest_threshold) {
				recent_scroll_events.pop_front();
			} else {
				break;
			}
		}
		return;
	}

	// Stop event. Potentially engage the kinetic scroll at the end.

	defer _([&]() {
		// Clear the scroll events on return.
		recent_scroll_events.clear();
	});

	if (recent_scroll_events.empty()) {
		// No scroll events, nothing to do.
		return;
	}

	double sum_delta_x = 0.0;
	double sum_delta_y = 0.0;
	size_t num_delta_x = 0;
	size_t num_delta_y = 0;

	wlr_event_pointer_axis* last_horizontal_event = nullptr;
	wlr_event_pointer_axis* last_vertical_event = nullptr;

	for (auto& e: recent_scroll_events) {
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

	average_delta_x = num_delta_x != 0 ? sum_delta_x / (double) num_delta_x : 0.0;
	average_delta_y = num_delta_y != 0 ? sum_delta_y / (double) num_delta_y : 0.0;
	// Pythagorean theorem.
	double average_delta = average_delta_x * average_delta_x + average_delta_y * average_delta_y;

	if (average_delta < 1.0 * 1.0) {
		// Insignificant scroll events.
		return;
	}

	if ((last_horizontal_event == nullptr || abs(last_horizontal_event->delta) < 1.0) &&
	    (last_vertical_event == nullptr || abs(last_vertical_event->delta) < 1.0)) {
		// The user most likely stopped scrolling to rest his fingers on the touchpad.
		return;
	}

	enabled = true;
	last_distance_x = 0;
	last_distance_y = 0;
	last_real_event_time = event->time_msec;
}

void KineticScrolling::apply_kinetic_scrolling(wlr_seat* seat) {
	if (not enabled) {
		return;
	}

	uint32_t now = current_time_milliseconds();

	// The bigger this value is, the lower the friction will be, so the kinetic scrolling will last longer.
	const double looseness = 200;

	auto curve = [=](double velocity, double t) {
		double total_distance = looseness * velocity;
		// A nice property of this formula is that the derivative at 0 is velocity regardless of the looseness.
		// https://www.desmos.com/calculator/u5skvfat1u
		return total_distance * (-exp(-velocity / total_distance * t) + 1);
	};

	double vel_x = abs(average_delta_x);
	double vel_y = abs(average_delta_y);

	double distance_x = vel_x != 0 ? curve(vel_x, (double) now - last_real_event_time) : 0;
	double distance_y = vel_y != 0 ? curve(vel_y, (double) now - last_real_event_time) : 0;

	double dx = distance_x - last_distance_x;
	double dy = distance_y - last_distance_y;

	if (dx * dx + dy * dy < 0.5 * 0.5) {
		enabled = false;
	}

	dx = average_delta_x < 0 ? -dx : dx;
	dy = average_delta_y < 0 ? -dy : dy;

	// FIXME: Without this divider there's a visible discontinuity in velocity. This value works
	// well enough but I need to investigate why.
	const double magical_divider = 7;
	dx /= magical_divider;
	dy /= magical_divider;

	last_distance_x = distance_x;
	last_distance_y = distance_y;

	// Send fake events to the application under the cursor.
	wlr_seat_pointer_notify_axis(seat,
	                             now, WLR_AXIS_ORIENTATION_HORIZONTAL, dx,
	                             0, WLR_AXIS_SOURCE_FINGER);
	wlr_seat_pointer_notify_frame(seat);

	wlr_seat_pointer_notify_axis(seat,
	                             now, WLR_AXIS_ORIENTATION_VERTICAL, dy,
	                             0, WLR_AXIS_SOURCE_FINGER);
	wlr_seat_pointer_notify_frame(seat);

	if (not enabled) {
		// Send stop events to the application. GTK uses this signal to change the cursor image back to the normal one.
		wlr_seat_pointer_notify_axis(seat,
		                             now, WLR_AXIS_ORIENTATION_HORIZONTAL, 0,
		                             0, WLR_AXIS_SOURCE_FINGER);

		wlr_seat_pointer_notify_frame(seat);
		wlr_seat_pointer_notify_axis(seat,
		                             now, WLR_AXIS_ORIENTATION_VERTICAL, 0,
		                             0, WLR_AXIS_SOURCE_FINGER);
		wlr_seat_pointer_notify_frame(seat);
	}
}

void KineticScrolling::stop() {
	enabled = false;
}
