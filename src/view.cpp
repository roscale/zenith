#include "view.hpp"
#include "server.hpp"
#include "platform_channels/encodable_value.h"
#include "messages.hpp"
#include "assert.hpp"
#include "xdg_surface_get_visible_bounds.hpp"

extern "C" {
#include <sys/epoll.h>
#define static
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/gles2.h>
#undef static
}

using namespace flutter;

static size_t next_view_id = 1;

ZenithSurface::ZenithSurface(wlr_surface* surface) : surface{surface}, id{next_view_id++} {
	commit.notify = surface_commit;
	wl_signal_add(&surface->events.commit, &commit);

	new_subsurface.notify = surface_new_subsurface;
	wl_signal_add(&surface->events.new_subsurface, &new_subsurface);

	destroy.notify = surface_destroy;
	wl_signal_add(&surface->events.destroy, &destroy);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"

ZenithXdgSurface::ZenithXdgSurface(wlr_xdg_surface* xdg_surface) : xdg_surface{xdg_surface} {
	destroy.notify = xdg_surface_destroy2;
	wl_signal_add(&xdg_surface->events.destroy, &destroy);

	map.notify = xdg_surface_map2;
	wl_signal_add(&xdg_surface->events.map, &map);

	unmap.notify = xdg_surface_unmap2;
	wl_signal_add(&xdg_surface->events.unmap, &unmap);

	switch (xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_NONE:
			assert(false && "unreachable");
			break;
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			toplevel = new ZenithXdgToplevel(xdg_surface->toplevel);
			auto* server = ZenithServer::instance();
			server->toplevels.insert(std::make_pair(zenith_surface()->id, toplevel));
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP:
			popup = new ZenithXdgPopup(xdg_surface->popup);
			break;
	}
}

ZenithSurface* ZenithXdgSurface::zenith_surface() const {
	return static_cast<ZenithSurface*>(xdg_surface->surface->data);
}

#pragma clang diagnostic pop

void surface_commit(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, commit);
	wlr_surface* surface = zenith_surface->surface;

	SurfaceCommitMessage commit_message{};
	commit_message.view_id = zenith_surface->id;

	wlr_texture* texture = wlr_surface_get_texture(zenith_surface->surface);
	int texture_id;
	if (texture != nullptr && wlr_texture_is_gles2(texture)) {
		// Wait for the texture to fully render, otherwise we'll see visual artifacts.
		glFinish();

		wlr_gles2_texture_attribs attribs{};
		wlr_gles2_texture_get_attribs(texture, &attribs);
		texture_id = (int) attribs.tex;

		FlutterEngineRegisterExternalTexture(ZenithServer::instance()->embedder_state->engine, texture_id);
		FlutterEngineMarkExternalTextureFrameAvailable(ZenithServer::instance()->embedder_state->engine, texture_id);
	} else {
		texture_id = -1;
	}

	SurfaceRole role;
	if (wlr_surface_is_xdg_surface(surface)) {
		role = XDG_SURFACE;
	} else if (wlr_surface_is_subsurface(surface)) {
		role = SUBSURFACE;
	} else {
		role = NONE;
	}

	commit_message.surface = {
		  .role = role,
		  .texture_id = texture_id,
		  .x = surface->sx,
		  .y = surface->sy,
		  .width = surface->current.width,
		  .height = surface->current.height,
		  .scale = surface->current.scale,
		  .input_region = surface->input_region.extents,
	};

	std::vector<SubsurfaceParentState> below{};
	std::vector<SubsurfaceParentState> above{};

	struct wlr_subsurface* subsurface;
	wl_list_for_each(subsurface, &surface->current.subsurfaces_below, current.link) {
		auto* subsurface_surface = static_cast<ZenithSurface*>(subsurface->surface->data);
		SubsurfaceParentState state = {
			  .id = (int64_t) subsurface_surface->id,
			  .x = subsurface->current.x,
			  .y = subsurface->current.y,
		};
		below.push_back(state);
	}
	wl_list_for_each(subsurface, &surface->current.subsurfaces_above, current.link) {
		auto* subsurface_surface = static_cast<ZenithSurface*>(subsurface->surface->data);
		SubsurfaceParentState state = {
			  .id = (int64_t) subsurface_surface->id,
			  .x = subsurface->current.x,
			  .y = subsurface->current.y,
		};
		above.push_back(state);
	}

	commit_message.subsurfaces_below = std::move(below);
	commit_message.subsurfaces_above = std::move(above);

	if (role == XDG_SURFACE) {
		wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);
		wlr_box visible_bounds = xdg_surface_get_visible_bounds(xdg_surface);
		commit_message.xdg_surface = {
			  .role = xdg_surface->role,
			  .x = visible_bounds.x,
			  .y = visible_bounds.y,
			  .width = visible_bounds.width,
			  .height = visible_bounds.height,
		};

		if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
			wlr_xdg_popup* popup = xdg_surface->popup;
			int64_t parent_id;
			if (popup->parent != nullptr) {
				assert(wlr_surface_is_xdg_surface(popup->parent));
				auto* parent = static_cast<ZenithSurface*>(popup->parent->data);
				parent_id = (int64_t) parent->id;
			} else {
				parent_id = 0;
			}

			const wlr_box& geometry = popup->geometry;
			commit_message.xdg_popup = {
				  .parent_id = parent_id,
				  .x = geometry.x,
				  .y = geometry.y,
				  .width = geometry.width,
				  .height = geometry.height,
			};
		}
	}

	BinaryMessenger& messenger = ZenithServer::instance()->embedder_state->messenger;
	send_surface_commit(messenger, commit_message);
}

void surface_new_subsurface(wl_listener* listener, void* data) {
	ZenithSurface* surface = wl_container_of(listener, surface, new_subsurface);
	auto* subsurface = static_cast<wlr_subsurface*>(data);
	subsurface->data = new ZenithSubsurface(subsurface);
}

void surface_destroy(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, destroy);
	bool erased = ZenithServer::instance()->surfaces.erase(zenith_surface->id);
	assert(erased);
	// TODO: Send destroy to Flutter.
	delete zenith_surface;
}

void xdg_surface_map2(wl_listener* listener, void* data) {
	ZenithXdgSurface* zenith_xdg_surface = wl_container_of(listener, zenith_xdg_surface, map);
	BinaryMessenger& messenger = ZenithServer::instance()->embedder_state->messenger;
	auto* surface = static_cast<ZenithSurface*>(zenith_xdg_surface->xdg_surface->surface->data);

	send_xdg_surface_map(messenger, surface->id);
}

void xdg_surface_unmap2(wl_listener* listener, void* data) {
	ZenithXdgSurface* zenith_xdg_surface = wl_container_of(listener, zenith_xdg_surface, unmap);
	BinaryMessenger& messenger = ZenithServer::instance()->embedder_state->messenger;
	auto* surface = static_cast<ZenithSurface*>(zenith_xdg_surface->xdg_surface->surface->data);

	// TODO: unlock after the animations are done.
//	wlr_buffer_lock(surface->surface->buffer->source);
//	wlr_buffer_lock(&surface->surface->buffer->base);

	send_xdg_surface_unmap(messenger, surface->id);
}

void xdg_surface_destroy2(wl_listener* listener, void* data) {
	ZenithXdgSurface* zenith_xdg_surface = wl_container_of(listener, zenith_xdg_surface, destroy);
	ZenithSurface* zenith_surface = zenith_xdg_surface->zenith_surface();
	auto* server = ZenithServer::instance();

	if (zenith_xdg_surface->xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		bool erased = server->toplevels.erase(zenith_surface->id);
		assert(erased);
	}
	bool erased = server->xdg_surfaces.erase(zenith_surface->id);
	assert(erased);

	if (zenith_xdg_surface->xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		delete zenith_xdg_surface->toplevel;
	} else if (zenith_xdg_surface->xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
		delete zenith_xdg_surface->popup;
	}

	zenith_xdg_surface->xdg_surface->data = nullptr;
	delete zenith_xdg_surface;
}

ZenithXdgToplevel::ZenithXdgToplevel(wlr_xdg_toplevel* xdg_toplevel) : xdg_toplevel{xdg_toplevel} {
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

ZenithXdgSurface* ZenithXdgToplevel::zenith_xdg_surface() const {
	return static_cast<ZenithXdgSurface*>(xdg_toplevel->base->data);
}

ZenithXdgPopup::ZenithXdgPopup(wlr_xdg_popup* xdg_popup) : xdg_popup{xdg_popup} {

}

ZenithXdgSurface* ZenithXdgPopup::zenith_xdg_surface() const {
	return static_cast<ZenithXdgSurface*>(xdg_popup->base->data);
}

ZenithSubsurface::ZenithSubsurface(wlr_subsurface* subsurface) : subsurface{subsurface} {
	map.notify = subsurface_map;
	wl_signal_add(&subsurface->events.map, &map);

	unmap.notify = subsurface_unmap;
	wl_signal_add(&subsurface->events.unmap, &unmap);

	destroy.notify = subsurface_destroy;
	wl_signal_add(&subsurface->events.destroy, &destroy);
}

ZenithSurface* ZenithSubsurface::zenith_surface() const {
	return static_cast<ZenithSurface*>(subsurface->surface->data);
}

void subsurface_map(wl_listener* listener, void* data) {
	ZenithSubsurface* zenith_subsurface = wl_container_of(listener, zenith_subsurface, map);
	BinaryMessenger& messenger = ZenithServer::instance()->embedder_state->messenger;
	send_subsurface_map(messenger, zenith_subsurface->zenith_surface()->id);
}

void subsurface_unmap(wl_listener* listener, void* data) {
	ZenithSubsurface* zenith_subsurface = wl_container_of(listener, zenith_subsurface, unmap);
	BinaryMessenger& messenger = ZenithServer::instance()->embedder_state->messenger;
	send_subsurface_unmap(messenger, zenith_subsurface->zenith_surface()->id);
}

void subsurface_destroy(wl_listener* listener, void* data) {
	ZenithSubsurface* zenith_subsurface = wl_container_of(listener, zenith_subsurface, destroy);
	zenith_subsurface->subsurface->data = nullptr;
	delete zenith_subsurface;
}
