#include "zenith_xdg_toplevel.hpp"
#include "server.hpp"

ZenithXdgToplevel::ZenithXdgToplevel(wlr_xdg_toplevel* xdg_toplevel,
                                     std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface)
	  : xdg_toplevel{xdg_toplevel}, zenith_xdg_surface(std::move(zenith_xdg_surface)) {

	auto* server = ZenithServer::instance();
	if (server->start_windows_maximized) {
		resize(server->max_window_size.width, server->max_window_size.height);
		maximize(true);
	}

	request_fullscreen.notify = zenith_xdg_toplevel_request_fullscreen;
	wl_signal_add(&xdg_toplevel->events.request_fullscreen, &request_fullscreen);

	set_app_id.notify = zenith_xdg_toplevel_set_app_id;
	wl_signal_add(&xdg_toplevel->events.set_app_id, &set_app_id);

	request_move.notify = zenith_xdg_toplevel_request_move;
	wl_signal_add(&xdg_toplevel->events.request_move, &request_move);

	request_resize.notify = zenith_xdg_toplevel_request_resize;
	wl_signal_add(&xdg_toplevel->events.request_resize, &request_resize);
}

void ZenithXdgToplevel::focus() const {
	auto* server = ZenithServer::instance();
	wlr_seat* seat = server->seat;
	wlr_surface* prev_surface = seat->keyboard_state.focused_surface;
	wlr_xdg_surface* base_xdg_surface = xdg_toplevel->base;

	bool is_surface_already_focused = prev_surface == base_xdg_surface->surface;
	if (is_surface_already_focused) {
		return;
	}

	if (prev_surface != nullptr) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		wl_client* client = wl_resource_get_client(prev_surface->resource);
		for (auto& text_input: server->text_inputs) {
			if (wl_resource_get_client(text_input->wlr_text_input->resource) == client) {
				text_input->leave();
			}
		}

		wlr_xdg_surface* previous;
		if (wlr_surface_is_xdg_surface(prev_surface)
		    && (previous = wlr_xdg_surface_from_wlr_surface(prev_surface)) != nullptr) {
			// FIXME: There is some weirdness going on which requires this seemingly redundant check.
			// I think the surface might be already destroyed but in this case keyboard_state.focused_surface
			// should be automatically set to null according to wlroots source code.
			// It seems that it doesn't cause any more crashes but I don't think this is the right fix.
			wlr_xdg_toplevel_set_activated(previous, false);
		}
	}
	// Activate the new surface.
	wlr_xdg_toplevel_set_activated(base_xdg_surface, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);
	if (keyboard != nullptr) {
		wlr_seat_keyboard_enter(seat, base_xdg_surface->surface,
		                        keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);

		wl_client* client = wl_resource_get_client(base_xdg_surface->resource);
		for (auto& text_input: server->text_inputs) {
			if (wl_resource_get_client(text_input->wlr_text_input->resource) == client &&
			    text_input->wlr_text_input->focused_surface != base_xdg_surface->surface) {
				text_input->enter(base_xdg_surface->surface);
			}
		}
	}
}

void ZenithXdgToplevel::maximize(bool value) const {
	wlr_xdg_toplevel_set_maximized(xdg_toplevel->base, value);
}

void ZenithXdgToplevel::resize(size_t width, size_t height) const {
	wlr_xdg_toplevel_set_size(xdg_toplevel->base, width, height);
}

void zenith_xdg_toplevel_request_fullscreen(wl_listener* listener, void* data) {
	auto* event = static_cast<wlr_xdg_toplevel_set_fullscreen_event*>(data);
	wlr_xdg_toplevel_set_fullscreen(event->surface, event->fullscreen);
}

void zenith_xdg_toplevel_set_app_id(wl_listener* listener, void* data) {
	ZenithXdgToplevel* zenith_xdg_toplevel = wl_container_of(listener, zenith_xdg_toplevel, set_app_id);
	char* app_id = zenith_xdg_toplevel->zenith_xdg_surface->xdg_surface->toplevel->app_id;
}

void zenith_xdg_toplevel_request_move(wl_listener* listener, void* data) {
	ZenithXdgToplevel* zenith_xdg_toplevel = wl_container_of(listener, zenith_xdg_toplevel, request_move);
	size_t id = zenith_xdg_toplevel->zenith_xdg_surface->zenith_surface->id;
	ZenithServer::instance()->embedder_state->interactive_move(id);
}

void zenith_xdg_toplevel_request_resize(wl_listener* listener, void* data) {
	ZenithXdgToplevel* zenith_xdg_toplevel = wl_container_of(listener, zenith_xdg_toplevel, request_resize);
	auto* event = static_cast<wlr_xdg_toplevel_resize_event*>(data);
	auto edge = static_cast<xdg_toplevel_resize_edge>(event->edges);
	if (edge == XDG_TOPLEVEL_RESIZE_EDGE_NONE) {
		// I don't know how to interpret this event.
		return;
	}
	size_t id = zenith_xdg_toplevel->zenith_xdg_surface->zenith_surface->id;
	ZenithServer::instance()->embedder_state->interactive_resize(id, edge);
}
