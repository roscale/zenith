/// Backported functionality from newer versions of wlroots.

#include "wlr_extensions.hpp"
#include <wayland-client-protocol.h>
#include <wayland-server-protocol.h>
#include <wayland-server-core.h>

extern "C" {
#define static
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_surface.h>
#undef static
}

// Copied from wlroots.
static struct wlr_seat_client* seat_client_from_touch_resource(struct wl_resource* resource) {
	return static_cast<wlr_seat_client*>(wl_resource_get_user_data(resource));
}

// Copied from wlroots.
static void handle_noop(struct wl_listener* listener, void* data) {
	// Do nothing
}

// Copied from wlroots.
void wlr_signal_emit_safe(struct wl_signal* signal, void* data) {
	struct wl_listener cursor;
	struct wl_listener end;

	/* Add two special markers: one cursor and one end marker. This way, we know
	 * that we've already called listeners on the left of the cursor and that we
	 * don't want to call listeners on the right of the end marker. The 'it'
	 * function can remove any element it wants from the list without troubles.
	 * wl_list_for_each_safe tries to be safe but it fails: it works fine
	 * if the current item is removed, but not if the next one is. */
	wl_list_insert(&signal->listener_list, &cursor.link);
	cursor.notify = handle_noop;
	wl_list_insert(signal->listener_list.prev, &end.link);
	end.notify = handle_noop;

	while (cursor.link.next != &end.link) {
		struct wl_list* pos = cursor.link.next;
		struct wl_listener* l = wl_container_of(pos, l, link);

		wl_list_remove(&cursor.link);
		wl_list_insert(pos, &cursor.link);

		l->notify(l, data);
	}

	wl_list_remove(&cursor.link);
	wl_list_remove(&end.link);
}

// Copied from wlroots.
static void touch_point_clear_focus(struct wlr_touch_point* point) {
	if (point->focus_surface) {
		wl_list_remove(&point->focus_surface_destroy.link);
		point->focus_client = NULL;
		point->focus_surface = NULL;
	}
}

// Copied from wlroots.
static void touch_point_destroy(struct wlr_touch_point* point) {
	wlr_signal_emit_safe(&point->events.destroy, point);

	touch_point_clear_focus(point);
	wl_list_remove(&point->surface_destroy.link);
	wl_list_remove(&point->client_destroy.link);
	wl_list_remove(&point->link);
	free(point);
}

// Copied from wlroots 0.16.
void wlr_seat_touch_send_cancel(struct wlr_seat* seat, struct wlr_surface* surface) {
	struct wl_client* client = wl_resource_get_client(surface->resource);
	struct wlr_seat_client* seat_client = wlr_seat_client_for_wl_client(seat, client);
	if (seat_client == NULL) {
		return;
	}

	struct wl_resource* resource;
	wl_resource_for_each(resource, &seat_client->touches) {
		if (seat_client_from_touch_resource(resource) == NULL) {
			continue;
		}
		wl_touch_send_cancel(resource);
	}

}

// Copied from wlroots 0.16.
void wlr_seat_touch_notify_cancel(struct wlr_seat* seat, struct wlr_surface* surface) {
	struct wlr_seat_touch_grab* grab = seat->touch_state.grab;
	wlr_seat_touch_send_cancel(grab->seat, surface);

	struct wl_client* client = wl_resource_get_client(surface->resource);
	struct wlr_seat_client* seat_client = wlr_seat_client_for_wl_client(seat, client);
	if (seat_client == NULL) {
		return;
	}
	struct wlr_touch_point* point, * tmp;
	wl_list_for_each_safe(point, tmp, &seat->touch_state.touch_points, link) {
		if (point->client == seat_client) {
			touch_point_destroy(point);
		}
	}
}