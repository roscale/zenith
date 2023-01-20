#include "zenith_surface.hpp"
#include "server.hpp"
#include "messages.hpp"
#include "xdg_surface_get_visible_bounds.hpp"
#include "time.hpp"
#include "scoped_wlr_buffer.hpp"

extern "C" {
#define static
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/gles2.h>
#undef static
}

static size_t next_view_id = 1;

ZenithSurface::ZenithSurface(wlr_surface* surface) : surface{surface}, id{next_view_id++} {
	commit.notify = zenith_surface_commit;
	wl_signal_add(&surface->events.commit, &commit);

	new_subsurface.notify = zenith_subsurface_create;
	wl_signal_add(&surface->events.new_subsurface, &new_subsurface);

	destroy.notify = zenith_surface_destroy;
	wl_signal_add(&surface->events.destroy, &destroy);
}

void zenith_surface_create(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_surface);
	auto* surface = static_cast<wlr_surface*>(data);
	auto* zenith_surface = new ZenithSurface(surface);
	surface->data = zenith_surface;
	server->surfaces.insert(std::make_pair(zenith_surface->id, zenith_surface));
}

void zenith_surface_commit(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, commit);
	wlr_surface* surface = zenith_surface->surface;

	SurfaceCommitMessage commit_message{};
	commit_message.view_id = zenith_surface->id;

	wlr_texture* texture = wlr_surface_get_texture(zenith_surface->surface);
	if (texture != nullptr && wlr_texture_is_gles2(texture)) {
		// Wait for the texture to fully render, otherwise we'll see visual artifacts.
		glFinish();

		assert(surface->buffer != nullptr);
		auto* server = ZenithServer::instance();

		std::shared_ptr<DoubleBuffering<wlr_buffer>> buffer_chain;
		auto it = server->surface_buffer_chains.find(zenith_surface->id);
		if (it == server->surface_buffer_chains.end()) {
			buffer_chain = std::make_shared<DoubleBuffering<wlr_buffer>>();
			server->surface_buffer_chains.insert(std::pair(zenith_surface->id, buffer_chain));
			FlutterEngineRegisterExternalTexture(server->embedder_state->engine, (int64_t) zenith_surface->id);
		} else {
			buffer_chain = it->second;
			FlutterEngineMarkExternalTextureFrameAvailable(server->embedder_state->engine,
			                                               (int64_t) zenith_surface->id);
		}
		buffer_chain->commit_new_buffer(scoped_wlr_buffer(&surface->buffer->base));

		wlr_gles2_texture_attribs attribs{};
		wlr_gles2_texture_get_attribs(texture, &attribs);
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
		  .texture_id = (int) zenith_surface->id,
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

void zenith_surface_destroy(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, destroy);
	auto* server = ZenithServer::instance();
	server->surface_buffer_chains.erase(zenith_surface->id);
	bool erased = server->surfaces.erase(zenith_surface->id);
	assert(erased);
	// TODO: Send destroy to Flutter.
}
