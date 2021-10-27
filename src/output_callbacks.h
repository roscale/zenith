#pragma once

#include <wayland-server.h>

void server_new_output(struct wl_listener* listener, void* data);

void output_frame(struct wl_listener* listener, void* data);
