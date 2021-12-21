#pragma once

#include <cstddef>

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithServer;

struct ZenithView {
	ZenithView(ZenithServer* server, wlr_xdg_surface* xdg_surface);

	/*
	 * Activates a view and make the keyboard enter the surface of the view.
	 */
	void focus();

	ZenithServer* server;
	wlr_xdg_surface* xdg_surface;
	size_t id;
	bool mapped;
	int x, y;
	/* callbacks */
	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};
};

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
