#pragma once

#include <cstddef>

extern "C" {
#include "wlr/types/wlr_xdg_shell.h"
}

struct ZenithServer;

struct ZenithView {
	ZenithView(ZenithServer* server, wlr_surface* surface);

	ZenithServer* server;
	wlr_surface* surface;
	size_t id;
	bool mapped;

	/*
	 * Activate the view and make the keyboard enter the surface of this view.
	 */
	virtual void focus() = 0;

	virtual void close() = 0;

	virtual void pointer_hover(double x, double y) = 0;

	virtual void resize(uint32_t width, uint32_t height) = 0;
};
