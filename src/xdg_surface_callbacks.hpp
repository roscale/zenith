#pragma once

#include <wayland-server.h>

/*
 * This event is raised when wlr_xdg_shell receives a new xdg surface from a
 * client, either a toplevel (application window) or popup.
 */
void server_new_xdg_surface(wl_listener* listener, void* data);

/*
 * Called when the surface is mapped, or ready to display on-screen.
 */
void xdg_surface_map(wl_listener* listener, void* data);

/*
 * Called when the surface is unmapped, and should no longer be shown.
 */
void xdg_surface_unmap(wl_listener* listener, void* data);

/*
 * Called when the surface is destroyed and should never be shown again.
 */
void xdg_surface_destroy(wl_listener* listener, void* data);
