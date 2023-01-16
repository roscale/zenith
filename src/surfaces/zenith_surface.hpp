#pragma once

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithTextInput;

struct ZenithSurface {
	explicit ZenithSurface(wlr_surface* surface);

	wlr_surface* surface;
	size_t id;
	ZenithTextInput* active_text_input = {};

	/* callbacks */
	wl_listener commit{};
	wl_listener new_subsurface{};
	wl_listener destroy{};
};

void zenith_surface_create(wl_listener* listener, void* data);

void zenith_surface_commit(wl_listener* listener, void* data);

void zenith_surface_destroy(wl_listener* listener, void* data);
