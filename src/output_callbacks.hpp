#pragma once

#include <wayland-server.h>

/*
 * This event is raised when a new output is detected, like a monitor or a projector.
 */
void server_new_output(wl_listener* listener, void* data);

/*
 * This function is called every time an output is ready to display a frame, generally at the output's refresh rate.
 */
void output_frame(wl_listener* listener, void* data);
