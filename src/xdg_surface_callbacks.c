#include "xdg_surface_callbacks.h"
#include "flutland_structs.h"

#include <wayland-util.h>

#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <wlr/types/wlr_xdg_shell.h>

void xdg_surface_map(struct wl_listener* listener, void* data) {
	printf("MAP SURFACE\n");
	fflush(stdout);
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct flutland_view* view = wl_container_of(listener, view, map);
	view->mapped = true;
}

void xdg_surface_unmap(struct wl_listener* listener, void* data) {
	printf("UNMAP SURFACE\n");
	fflush(stdout);
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct flutland_view* view = wl_container_of(listener, view, unmap);
	view->mapped = false;
}

void xdg_surface_destroy(struct wl_listener* listener, void* data) {
	printf("DESTROY SURFACE\n");
	fflush(stdout);
	/* Called when the surface is destroyed and should never be shown again. */
	struct flutland_view* view = wl_container_of(listener, view, destroy);
	wl_list_remove(&view->link);
	free(view);
}

void server_new_xdg_surface(struct wl_listener* listener, void* data) {
	/* This event is raised when wlr_xdg_shell receives a new xdg surface from a
	 * client, either a toplevel (application window) or popup. */
	struct flutland_server* server =
		  wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface* xdg_surface = data;
	if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		return;
	}

	/* Allocate a flutland_view for this surface */
	struct flutland_view* view =
		  calloc(1, sizeof(struct flutland_view));
	view->server = server;
	view->xdg_surface = xdg_surface;

	/* Listen to the various events it can emit */
	view->map.notify = xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	/* Add it to the list of views. */
	wl_list_insert(&server->views, &view->link);

	printf("\nNEW CLIENT\n\n");
}