#pragma once

#include "zenith_xdg_surface.hpp"

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithXdgToplevel {
	ZenithXdgToplevel(wlr_xdg_toplevel* xdg_toplevel,
	                  std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface);

	wlr_xdg_toplevel* xdg_toplevel;
	std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface;
	bool visible = true;

	/* callbacks */
	wl_listener request_fullscreen = {};
	wl_listener set_app_id = {};
	// TODO: set_title, etc.

	void focus() const;

	void maximize() const;
};

void zenith_xdg_toplevel_request_fullscreen(wl_listener* listener, void* data);

void zenith_xdg_toplevel_set_app_id(wl_listener* listener, void* data);
