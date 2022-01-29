#pragma once

#include <deque>

extern "C" {
#include <wlr/types/wlr_pointer.h>
}

class KineticScrolling {
	bool enabled = false;
	std::deque<wlr_event_pointer_axis> recent_scroll_events{};
};