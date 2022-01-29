#pragma once

#include <deque>

extern "C" {
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
}

class KineticScrolling {
	bool enabled = false;
	std::deque<wlr_event_pointer_axis> recent_scroll_events{};
	uint32_t last_real_event_time{};
	double average_delta_x{};
	double average_delta_y{};
	double last_distance_x{};
	double last_distance_y{};

public:
	void record_scroll_event(wlr_event_pointer_axis* event);

	void apply_kinetic_scrolling(wlr_seat* seat);

	void stop();
};