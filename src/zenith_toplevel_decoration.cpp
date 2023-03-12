#include <memory>
#include <utility>
#include "zenith_toplevel_decoration.hpp"
#include "server.hpp"

ZenithToplevelDecoration::ZenithToplevelDecoration(wlr_xdg_toplevel_decoration_v1* wlr_toplevel_decoration,
                                                   std::shared_ptr<ZenithXdgSurface> xdg_surface)
	  : wlr_toplevel_decoration(wlr_toplevel_decoration), xdg_surface(std::move(xdg_surface)) {

	request_mode.notify = request_mode_handle;
	wl_signal_add(&wlr_toplevel_decoration->events.request_mode, &request_mode);

	destroy.notify = destroy_handle;
	wl_signal_add(&wlr_toplevel_decoration->events.destroy, &destroy);
}

void toplevel_decoration_create_handle(wl_listener* listener, void* data) {
	auto* wlr_toplevel_decoration = static_cast<wlr_xdg_toplevel_decoration_v1*>(data);
	auto* server = ZenithServer::instance();

	auto* xdg_surface_ptr = static_cast<ZenithXdgSurface*>(wlr_toplevel_decoration->surface->data);
	size_t id = xdg_surface_ptr->zenith_surface->id;
	std::shared_ptr<ZenithXdgSurface> xdg_surface = server->xdg_surfaces.at(id);

	auto toplevel_decoration = std::make_shared<ZenithToplevelDecoration>(wlr_toplevel_decoration, xdg_surface);

	ZenithServer::instance()->toplevel_decorations.insert(
		  std::make_pair(xdg_surface->zenith_surface->id, toplevel_decoration));

	request_mode_handle(&toplevel_decoration->request_mode, nullptr);
}

void request_mode_handle(wl_listener* listener, void* data) {
	ZenithToplevelDecoration* toplevel_decoration = wl_container_of(listener, toplevel_decoration, request_mode);
	wlr_xdg_toplevel_decoration_v1* wlr_toplevel_decoration = toplevel_decoration->wlr_toplevel_decoration;
	wlr_xdg_toplevel_decoration_v1_mode requested_mode = wlr_toplevel_decoration->requested_mode;

	std::cout << "req " << requested_mode << std::endl;

	if (requested_mode != WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_NONE) {
		wlr_xdg_toplevel_decoration_v1_set_mode(wlr_toplevel_decoration, requested_mode);
	} else {
		// If the app doesn't have a preferred decoration mode, we'll just pick server side.
		wlr_xdg_toplevel_decoration_v1_set_mode(wlr_toplevel_decoration,
		                                        WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}
}

void destroy_handle(wl_listener* listener, void* data) {
	ZenithToplevelDecoration* toplevel_decoration = wl_container_of(listener, toplevel_decoration, destroy);
	ZenithServer::instance()->toplevel_decorations.erase(toplevel_decoration->xdg_surface->zenith_surface->id);
}
