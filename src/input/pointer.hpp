#pragma once

#include <queue>
#include "util/embedder/mouse_button_tracker.hpp"

extern "C" {
#include "wlr/types/wlr_cursor.h"
#include "wlr/types/wlr_pointer.h"
#define static
#include "wlr/types/wlr_xcursor_manager.h"
#undef static
}

struct ZenithServer;

struct ZenithPointer {
	explicit ZenithPointer(ZenithServer* server);

	ZenithServer* server;
	wlr_cursor* cursor;
	wlr_xcursor_manager* cursor_mgr;
	MouseButtonTracker mouse_button_tracker{};
	/* callbacks */
	wl_listener cursor_motion{};
	wl_listener cursor_motion_absolute{};
	wl_listener cursor_button{};
	wl_listener cursor_axis{};
	wl_listener cursor_frame{};

private:
	bool visible = true;

public:
	bool is_visible() const {
		return visible;
	}

	void set_visible(bool value) {
		if (visible == value) {
			return;
		}
		visible = value;
		if (visible) {
			wlr_xcursor_manager_set_cursor_image(cursor_mgr, "left_ptr", cursor);
		} else {
			wlr_cursor_set_surface(cursor, nullptr, 0, 0);
		}
	}
};

/*
 * The cursor forwards this event when a pointer emits a _relative_ pointer motion event (i.e. a delta).
 */
void server_cursor_motion(wl_listener* listener, void* data);

/*
 * The cursor forwards this event when a pointer emits an _absolute_
 * motion event, from 0..1 on each axis. This happens, for example, when
 * wlroots is running under a Wayland window rather than KMS+DRM, and you
 * move the mouse over the window. You could enter the window from any edge,
 * so we have to warp the mouse there. There is also some hardware which
 * emits these events.
 */
void server_cursor_motion_absolute(wl_listener* listener, void* data);

/*
 * The cursor forwards this event when a pointer emits a button event.
 */
void server_cursor_button(wl_listener* listener, void* data);

/*
 * The cursor forwards this event when a pointer emits an axis event, for example when you move the scroll wheel.
 */
void server_cursor_axis(wl_listener* listener, void* data);

/*
 * The cursor forwards this event when a pointer emits a frame
 * event. Frame events are sent after regular pointer events to group
 * multiple events together. For instance, two axis events may happen at the
 * same time, in which case a frame event won't be sent in between.
 */
void server_cursor_frame(wl_listener* listener, void* data);
