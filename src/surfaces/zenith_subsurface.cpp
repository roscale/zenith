#include "zenith_subsurface.hpp"
#include "binary_messenger.hpp"
#include "server.hpp"

ZenithSubsurface::ZenithSubsurface(wlr_subsurface* subsurface, std::shared_ptr<ZenithSurface> zenith_surface)
	  : subsurface{subsurface}, zenith_surface{std::move(zenith_surface)} {
	map.notify = zenith_subsurface_map;
	wl_signal_add(&subsurface->events.map, &map);

	unmap.notify = zenith_subsurface_unmap;
	wl_signal_add(&subsurface->events.unmap, &unmap);

	destroy.notify = zenith_subsurface_destroy;
	wl_signal_add(&subsurface->events.destroy, &destroy);
}

void zenith_subsurface_create(wl_listener* listener, void* data) {
	auto* server = ZenithServer::instance();
	auto* subsurface = static_cast<wlr_subsurface*>(data);
	auto* zenith_surface = static_cast<ZenithSurface*>(subsurface->surface->data);
	const std::shared_ptr<ZenithSurface>& zenith_surface_ref = server->surfaces.at(zenith_surface->id);

	auto* zenith_subsurface = new ZenithSubsurface(subsurface, zenith_surface_ref);
	subsurface->data = zenith_subsurface;
	server->subsurfaces.insert(std::make_pair(zenith_surface->id, zenith_subsurface));
}

void zenith_subsurface_map(wl_listener* listener, void* data) {
	ZenithSubsurface* zenith_subsurface = wl_container_of(listener, zenith_subsurface, map);
	size_t id = zenith_subsurface->zenith_surface->id;
	ZenithServer::instance()->embedder_state->map_subsurface(id);
}

void zenith_subsurface_unmap(wl_listener* listener, void* data) {
	ZenithSubsurface* zenith_subsurface = wl_container_of(listener, zenith_subsurface, unmap);
	size_t id = zenith_subsurface->zenith_surface->id;
	ZenithServer::instance()->embedder_state->unmap_subsurface(id);
}

void zenith_subsurface_destroy(wl_listener* listener, void* data) {
	auto* subsurface = static_cast<wlr_subsurface*>(data);
	ZenithSubsurface* zenith_subsurface = static_cast<ZenithSubsurface*>(subsurface->data);
	bool erased = ZenithServer::instance()->subsurfaces.erase(zenith_subsurface->zenith_surface->id);
	assert(erased);
}
