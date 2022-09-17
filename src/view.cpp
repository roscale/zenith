#include "view.hpp"
#include "server.hpp"
#include "platform_channels/encodable_value.h"
#include "standard_method_codec.h"
#include "messages.hpp"
#include "assert.hpp"
#include "framebuffer.hpp"
#include "xdg_surface_get_visible_bounds.hpp"

extern "C" {
#define static
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/gles2.h>
#undef static
}

using namespace flutter;

static size_t next_view_id = 1;
static size_t next_texture_id = 1;

ZenithView::ZenithView(ZenithServer* server, wlr_xdg_surface* xdg_surface)
	  : server(server), xdg_surface(xdg_surface), id(next_view_id++) {

	xdg_surface->data = this;

	/* Listen to the various events it can emit */
	map.notify = xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &map);

	unmap.notify = xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &unmap);

	destroy.notify = xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &destroy);

	commit.notify = xdg_surface_commit;
	wl_signal_add(&xdg_surface->surface->events.commit, &commit);

	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		// Tell the client the size we want before mapping the surface, to avoid having to resize
		// after being mapped which would result in a flicker.
		// FIXME
		// Obidian (Chromium-based) crashes with the error "xdg_surface has never been configured"
		maximize();
	}
}

void ZenithView::focus() const {
	ASSERT(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL, "Popups cannot get focus");

	wlr_seat* seat = server->seat;
	wlr_surface* prev_surface = seat->keyboard_state.focused_surface;

	bool is_surface_already_focused = prev_surface == xdg_surface->surface;
	if (is_surface_already_focused) {
		return;
	}

	if (prev_surface != nullptr) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		wlr_xdg_surface* previous;
		if (wlr_surface_is_xdg_surface(prev_surface)
		    && (previous = wlr_xdg_surface_from_wlr_surface(prev_surface)) != nullptr) {
			// FIXME: There is some weirdness going on which requires this seemingly redundant check.
			// I think the surface might be already destroyed but in this case keyboard_state.focused_surface
			// should be automatically set to null according to wlroots source code.
			// It seems that it doesn't cause any more crashes but I don't think this is the right fix.

			wl_client* client = wl_resource_get_client(prev_surface->resource);
			for (auto& text_input: server->text_inputs) {
				if (wl_resource_get_client(text_input->wlr_text_input->resource) == client) {
					text_input->leave();
				}
			}

			wlr_xdg_toplevel_set_activated(previous, false);
		}
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
		wlr_seat_keyboard_enter(seat, xdg_surface->surface,
		                        keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);

		wl_client* client = wl_resource_get_client(xdg_surface->resource);
		for (auto& text_input: server->text_inputs) {
			if (wl_resource_get_client(text_input->wlr_text_input->resource) == client &&
			    text_input->wlr_text_input->focused_surface != xdg_surface->surface) {
				text_input->enter(xdg_surface);
			}
		}
	}
}

void ZenithView::maximize() const {
	ASSERT(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL,
	       "View " << id << " cannot be maximized because it's not a top-level surface.");

	wlr_xdg_toplevel_set_size(xdg_surface, server->max_window_size.width, server->max_window_size.height);
	wlr_xdg_toplevel_set_maximized(xdg_surface, true);
}

void xdg_surface_map(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, map);
	ZenithServer* server = view->server;
	wlr_xdg_surface* xdg_surface = view->xdg_surface;
	wlr_surface* surface = xdg_surface->surface;
	wlr_texture* texture = wlr_surface_get_texture(surface);
	assert(texture != nullptr);

	view->mapped = true;

	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		view->focus();
		view->maximize();
	}

	// Create a framebuffer for this xdg_surface.
	{
		wlr_egl_make_current(wlr_gles2_renderer_get_egl(server->renderer));
		size_t texture_id = next_texture_id++;
		auto framebuffer = std::make_shared<Framebuffer>(texture->width, texture->height);

		{
			std::scoped_lock lock(server->surface_framebuffers_mutex);
			server->surface_framebuffers.insert(
				  std::pair(
						texture_id,
						framebuffer
				  )
			);
		}

		view->active_texture = texture_id;

		// TODO: The texture shouldn't be registered yet because nothing has been rendered to it.
		FlutterEngineRegisterExternalTexture(server->embedder_state->engine, (int64_t) view->active_texture);
	}

	view->visible_bounds = xdg_surface_get_visible_bounds(xdg_surface);

	switch (xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			send_window_mapped(server->embedder_state->messenger,
			                   view->id,
			                   view->active_texture,
			                   surface->current.width,
			                   surface->current.height,
			                   view->visible_bounds);
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP: {
			wlr_xdg_popup* popup = xdg_surface->popup;

			view->popup_geometry = wlr_box{
				  .x = popup->geometry.x,
				  .y = popup->geometry.y,
				  .width = surface->current.width,
				  .height = surface->current.height,
			};

			assert(popup->parent != nullptr);
			wlr_xdg_surface* parent_xdg_surface = wlr_xdg_surface_from_wlr_surface(popup->parent);
			auto* parent_view = static_cast<ZenithView*>(parent_xdg_surface->data);

			send_popup_mapped(server->embedder_state->messenger,
			                  view->id,
			                  view->active_texture,
			                  parent_view->id,
			                  view->popup_geometry,
			                  view->visible_bounds);
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
}

void xdg_surface_commit(wl_listener* listener, void* data) {
	auto* surface = static_cast<wlr_surface*>(data);
	auto* server = ZenithServer::instance();

	wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);
	if (not xdg_surface->mapped) {
		return;
	}

	auto* view = static_cast<ZenithView*>(xdg_surface->data);

	std::optional<std::reference_wrapper<wlr_box>> new_visible_bounds{};
	std::optional<int> new_texture_id{};
	std::optional<std::pair<int, int>> new_surface_size{};
	std::optional<std::pair<int, int>> new_popup_position{};

	wlr_box surface_bounds = xdg_surface_get_visible_bounds(xdg_surface);

	if (view->visible_bounds.x != surface_bounds.x
	    or view->visible_bounds.y != surface_bounds.y
	    or view->visible_bounds.width != surface_bounds.width
	    or view->visible_bounds.height != surface_bounds.height) {

		view->visible_bounds = surface_bounds;
		new_visible_bounds = surface_bounds;
	}

	Size surface_framebuffer_size{};
	{
		std::shared_ptr<Framebuffer> surface_framebuffer;
		{
			std::scoped_lock lock(server->surface_framebuffers_mutex);
			surface_framebuffer = server->surface_framebuffers.at(view->active_texture);
		}
		std::scoped_lock lock(surface_framebuffer->mutex);
		surface_framebuffer_size = Size{
			  .width = surface_framebuffer->width,
			  .height = surface_framebuffer->height,
		};
	}

	if ((size_t) surface->current.buffer_width != surface_framebuffer_size.width
	    or (size_t) surface->current.buffer_height != surface_framebuffer_size.height) {
		// Create a new framebuffer with the new size.
		wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
		wlr_egl_make_current(egl);

		size_t texture_id = next_texture_id++;
		auto framebuffer = std::make_shared<Framebuffer>(
			  surface->current.buffer_width,
			  surface->current.buffer_height
		);

		{
			std::scoped_lock lock(server->surface_framebuffers_mutex);
			server->surface_framebuffers.insert(
				  std::pair(
						texture_id,
						framebuffer
				  )
			);
		}

		view->active_texture = texture_id;

		// TODO: The texture shouldn't be registered yet because nothing has been rendered to it.
		// It causes some flickers on rare occasions when the surface is resized.
		FlutterEngineRegisterExternalTexture(server->embedder_state->engine, (int64_t) texture_id);

		new_surface_size = {surface->current.width, surface->current.height};
		new_texture_id = texture_id;
	}

	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
		wlr_box popup_box = {
			  .x = xdg_surface->popup->geometry.x,
			  .y = xdg_surface->popup->geometry.y,
			  .width = surface->current.buffer_width,
			  .height = surface->current.buffer_height,
		};
		if (popup_box.x != view->popup_geometry.x or popup_box.y != view->popup_geometry.y) {
			view->popup_geometry = popup_box;
			new_popup_position = {popup_box.x, popup_box.y};
		}
	}

	if (new_visible_bounds or new_surface_size or new_popup_position) {
		send_configure_xdg_surface(server->embedder_state->messenger,
		                           view->id, xdg_surface->role,
		                           new_visible_bounds,
		                           new_texture_id,
		                           new_surface_size,
		                           new_popup_position);
	}
}

void xdg_surface_unmap(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, unmap);
	view->mapped = false;

	switch (view->xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			send_window_unmapped(view->server->embedder_state->messenger, view->id);
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP: {
			send_popup_unmapped(view->server->embedder_state->messenger, view->id);
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
}

void xdg_surface_destroy(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, destroy);
	ZenithServer* server = view->server;

	bool erased = server->views.erase(view->id);
	assert(erased == 1);

	wl_list_remove(&view->map.link);
	wl_list_remove(&view->unmap.link);
	wl_list_remove(&view->destroy.link);
	wl_list_remove(&view->commit.link);

	delete view;
}
