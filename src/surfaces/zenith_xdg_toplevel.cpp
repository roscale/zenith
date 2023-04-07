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

	set_title.notify = zenith_xdg_toplevel_set_title;
	wl_signal_add(&xdg_toplevel->events.set_title, &set_title);

	request_move.notify = zenith_xdg_toplevel_request_move;
	wl_signal_add(&xdg_toplevel->events.request_move, &request_move);

	request_resize.notify = zenith_xdg_toplevel_request_resize;
	wl_signal_add(&xdg_toplevel->events.request_resize, &request_resize);
}

void ZenithXdgToplevel::focus(bool focus) const {
	auto* server = ZenithServer::instance();
	wlr_seat* seat = server->seat;
	wlr_xdg_surface* xdg_surface = xdg_toplevel->base;
	wlr_surface* surface = xdg_surface->surface;
	wl_client* client = wl_resource_get_client(xdg_surface->resource);

	if (focus) {
		wlr_surface* prev_surface = seat->keyboard_state.focused_surface;
		bool is_surface_already_focused = prev_surface == surface;
		if (is_surface_already_focused) {
			return;
		}
		// Activate the new surface.
		wlr_xdg_toplevel_set_activated(xdg_surface, true);
		/*
		 * Tell the seat to have the keyboard enter this surface. wlroots will keep
		 * track of this and automatically send key events to the appropriate
		 * clients without additional work on your part.
		 */
		wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);
		if (keyboard != nullptr) {
			wlr_seat_keyboard_notify_enter(seat, xdg_surface->surface,
			                               keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);

			for (auto& text_input: server->text_inputs) {
				if (wl_resource_get_client(text_input->wlr_text_input->resource) == client &&
				    text_input->wlr_text_input->focused_surface != xdg_surface->surface) {
					text_input->enter(xdg_surface->surface);
				}
			}
		}
	} else {
		for (auto& text_input: server->text_inputs) {
			if (wl_resource_get_client(text_input->wlr_text_input->resource) == client) {
				text_input->leave();
			}
		}
		wlr_seat_keyboard_notify_clear_focus(seat);
		wlr_xdg_toplevel_set_activated(xdg_toplevel->base, false);
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
	size_t id = zenith_xdg_toplevel->zenith_xdg_surface->zenith_surface->id;
	char* app_id = zenith_xdg_toplevel->zenith_xdg_surface->xdg_surface->toplevel->app_id;
	ZenithServer::instance()->embedder_state->set_app_id(id, app_id);
}

void zenith_xdg_toplevel_set_title(wl_listener* listener, void* data) {
	ZenithXdgToplevel* zenith_xdg_toplevel = wl_container_of(listener, zenith_xdg_toplevel, set_title);
	size_t id = zenith_xdg_toplevel->zenith_xdg_surface->zenith_surface->id;
	char* title = zenith_xdg_toplevel->zenith_xdg_surface->xdg_surface->toplevel->title;
	ZenithServer::instance()->embedder_state->set_window_title(id, title);
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
