#pragma once

#include <cstddef>

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithServer;
struct ZenithTextInput;

struct ZenithView {
	ZenithView(ZenithServer* server, wlr_xdg_surface* xdg_surface);

	/*
	 * Activate the view and make the keyboard enter the surface of this view.
	 */
	void focus() const;

	void maximize() const;

	ZenithServer* server;
	wlr_xdg_surface* xdg_surface;
	size_t id;
	bool mapped = false;
	bool visible = true;
	wlr_box popup_geometry{};
	wlr_box visible_bounds{};
	ZenithTextInput* active_text_input = nullptr;
	size_t active_texture = 0;

	/* callbacks */
	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};
	wl_listener commit{};
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
