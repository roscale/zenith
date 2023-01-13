#include <algorithm>
#include <iostream>
#include "wayland_event_tracker.hpp"
#include "time.hpp"

const uint64_t ONE_SEC = 1'000'000'000; // in nanoseconds

WaylandEventTracker* WaylandEventTracker::_instance = nullptr;

WaylandEventTracker& WaylandEventTracker::instance() {
	if (_instance == nullptr) {
		_instance = new WaylandEventTracker();
	}
	return *_instance;
}

struct WaylandEvent {
	uint64_t timestamp; // monotonic time in nanoseconds
	uint32_t serial;
	WaylandEventType type;
};

void WaylandEventTracker::trackEvent(uint64_t timestamp, uint32_t serial, WaylandEventType type) {
	cleanUpOldEvents();
	set.insert({timestamp, serial, type});
}

bool WaylandEventTracker::validateEvent(uint32_t serial, uint64_t allowed_types) {
	cleanUpOldEvents();
	auto it = std::find_if(set.cbegin(), set.cend(), [serial](const WaylandEvent& e) {
		return e.serial == serial;
	});
	if (it == set.end()) {
		return false;
	}
	return allowed_types & (uint64_t) it->type;
}

void WaylandEventTracker::cleanUpOldEvents() {
	uint64_t now = current_time_nanoseconds();
	auto it = set.cbegin();
	for (; it != set.cend() && now - it->timestamp > ONE_SEC; it++);
	set.erase(set.cbegin(), it); // Remove events older than one second.
}

bool wayland_event_compare::operator()(const WaylandEvent& e1, const WaylandEvent& e2) const {
	return e1.timestamp < e2.timestamp;
}
