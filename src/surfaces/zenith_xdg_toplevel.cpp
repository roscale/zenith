#include "zenith_xdg_toplevel.hpp"
#include "server.hpp"

ZenithXdgToplevel::ZenithXdgToplevel(wlr_xdg_toplevel* xdg_toplevel,
                                     std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface)
	  : xdg_toplevel{xdg_toplevel}, zenith_xdg_surface(std::move(zenith_xdg_surface)) {
	maximize();
	// TODO: Set up callbacks.
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

void ZenithXdgToplevel::maximize() const {
	auto* server = ZenithServer::instance();
	wlr_xdg_toplevel_set_size(xdg_toplevel->base, server->max_window_size.width, server->max_window_size.height);
	wlr_xdg_toplevel_set_maximized(xdg_toplevel->base, true);
}
