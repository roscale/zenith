#pragma once

#include <cstddef>

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithServer;

struct ZenithView {
	ZenithView(ZenithServer* server, wlr_xdg_surface* xdg_surface);

	/*
	 * Activate the view and make the keyboard enter the surface of this view.
	 */
	void focus();

	ZenithServer* server;
	wlr_xdg_surface* xdg_surface;
	size_t id;
	bool mapped;
	int x, y;
	wlr_box geometry{};
	/* callbacks */
	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};
	wl_listener commit{};
	wl_listener request_move{};
	wl_listener request_resize{};
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

/*
 * This event is raised when a client would like to begin an interactive
 * move, typically because the user clicked on their client-side
 * decorations. Note that a more sophisticated compositor should check the
 * provided serial against a list of button press serials sent to this
 * client, to prevent the client from requesting this whenever they want.
 */
void xdg_toplevel_request_move(wl_listener* listener, void* data);

/*
 * This event is raised when a client would like to begin an interactive
 * resize, typically because the user clicked on their client-side
 * decorations. Note that a more sophisticated compositor should check the
 * provided serial against a list of button press serials sent to this
 * client, to prevent the client from requesting this whenever they want.
 */
void xdg_toplevel_request_resize(wl_listener* listener, void* data);