#pragma once

#include <wayland-server.h>

void xdg_surface_map(struct wl_listener* listener, void* data);

void xdg_surface_unmap(struct wl_listener* listener, void* data);

void xdg_surface_destroy(struct wl_listener* listener, void* data);

void server_new_xdg_surface(struct wl_listener* listener, void* data);
