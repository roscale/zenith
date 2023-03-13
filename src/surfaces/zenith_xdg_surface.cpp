#include "zenith_xdg_surface.hpp"
#include "binary_messenger.hpp"
#include "server.hpp"
#include "assert.hpp"

ZenithXdgSurface::ZenithXdgSurface(wlr_xdg_surface* xdg_surface, std::shared_ptr<ZenithSurface> zenith_surface)
	  : xdg_surface{xdg_surface}, zenith_surface(std::move(zenith_surface)) {
	destroy.notify = zenith_xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &destroy);

	map.notify = zenith_xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &map);

	unmap.notify = zenith_xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &unmap);
}

void zenith_xdg_surface_create(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_xdg_surface);
	auto* xdg_surface = static_cast<wlr_xdg_surface*>(data);
	auto* zenith_surface = static_cast<ZenithSurface*>(xdg_surface->surface->data);
	const std::shared_ptr<ZenithSurface>& zenith_surface_ref = server->surfaces.at(zenith_surface->id);

	auto* zenith_xdg_surface = new ZenithXdgSurface(xdg_surface, zenith_surface_ref);
	xdg_surface->data = zenith_xdg_surface;
	auto zenith_xdg_surface_ref = std::shared_ptr<ZenithXdgSurface>(zenith_xdg_surface);
	server->xdg_surfaces.insert(std::make_pair(zenith_surface->id, zenith_xdg_surface_ref));

	switch (xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_NONE:
			ASSERT(false, "unreachable");
			break;
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			auto toplevel = new ZenithXdgToplevel(xdg_surface->toplevel, zenith_xdg_surface_ref);
			server->xdg_toplevels.insert(std::make_pair(zenith_surface->id, toplevel));
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP:
			auto popup = new ZenithXdgPopup(xdg_surface->popup, zenith_xdg_surface_ref);
			server->xdg_popups.insert(std::make_pair(zenith_surface->id, popup));
			break;
	}
}

void zenith_xdg_surface_map(wl_listener* listener, void* data) {
	ZenithXdgSurface* zenith_xdg_surface = wl_container_of(listener, zenith_xdg_surface, map);
	size_t id = zenith_xdg_surface->zenith_surface->id;
	ZenithServer::instance()->embedder_state->map_xdg_surface(id);
}

void zenith_xdg_surface_unmap(wl_listener* listener, void* data) {
	ZenithXdgSurface* zenith_xdg_surface = wl_container_of(listener, zenith_xdg_surface, unmap);
	size_t id = zenith_xdg_surface->zenith_surface->id;
	ZenithServer::instance()->embedder_state->unmap_xdg_surface(id);
}

void zenith_xdg_surface_destroy(wl_listener* listener, void* data) {
	auto* xdg_surface = static_cast<wlr_xdg_surface*>(data);
	auto zenith_xdg_surface = static_cast<ZenithXdgSurface*>(xdg_surface->data);
	size_t id = zenith_xdg_surface->zenith_surface->id;

//	ZenithXdgSurface* zenith_xdg_surface = wl_container_of(listener, zenith_xdg_surface, destroy);
	auto* server = ZenithServer::instance();

	if (zenith_xdg_surface->xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		bool erased = server->xdg_toplevels.erase(id);
		assert(erased);
	} else if (zenith_xdg_surface->xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
		bool erased = server->xdg_popups.erase(id);
		assert(erased);
	}
	bool erased = server->xdg_surfaces.erase(id);
	assert(erased);
}
