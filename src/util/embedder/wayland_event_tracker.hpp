#pragma once

/*
 * TODO:
 * This is currently unused because wlroots makes it difficult to obtain the serial of an event.
 * Some functions like `wlr_seat_touch_notify_down` return the serial as expected, but `wlr_seat_touch_notify_up`
 * doesn't. This is because when the function is called, wlroots chooses to either send an actual up event, or
 * finish a drag and drop operation if one is in progress. In the latter case, Wayland doesn't require a serial to be sent.
 * wlroots chose to design their API using the common denominator, so most functions don't return a serial.
 */

#include <queue>
#include <cstdint>
#include <set>

enum class WaylandEventType : uint64_t {
	Keyboard = 1 << 0,
	MouseButton = 1 << 1,
	Touch = 1 << 2,
};

struct WaylandEvent;

struct wayland_event_compare {
	bool operator()(const WaylandEvent& e1, const WaylandEvent& e2) const;
};

/*
 * The purpose of this class is to remember recent user input events and validate Wayland client requests.
 * For example, a Wayland client should be allowed to overwrite the clipboard only if the user clicked or
 * pressed some key inside the client's window.
 */
class WaylandEventTracker {
	static WaylandEventTracker* _instance;

	std::multiset<WaylandEvent, wayland_event_compare> set;

	void cleanUpOldEvents();

public:
	static WaylandEventTracker& instance();

	void trackEvent(uint64_t timestamp, uint32_t serial, WaylandEventType type);

	bool validateEvent(uint32_t serial, uint64_t allowed_types);
};
