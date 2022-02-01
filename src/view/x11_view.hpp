#pragma once

#include "view.hpp"

extern "C" {
#define static
#include "my_xwayland.h"
#undef static
}

struct ZenithX11View : public ZenithView {
	ZenithX11View(ZenithServer* server, wlr_xwayland_surface* x11_surface);

	wlr_xwayland_surface* x11_surface;

	/* callbacks */
	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};
	wl_listener request_move{};
	wl_listener request_resize{};
	wl_listener set_geometry{};

	void focus() override;

	void close() override;

	void pointer_hover(double x, double y) override;

	void resize(uint32_t width, uint32_t height) override;

	void render_to_fbo(GLuint fbo) override;
};

/*
 * Called when the surface is mapped, or ready to display on-screen.
 */
void xwayland_surface_map(wl_listener* listener, void* data);

/*
 * Called when the surface is unmapped, and should no longer be shown.
 */
void xwayland_surface_unmap(wl_listener* listener, void* data);

/*
 * Called when the surface is destroyed and should never be shown again.
 */
void xwayland_surface_destroy(wl_listener* listener, void* data);

void xwayland_surface_request_move(wl_listener* listener, void* data);

void xwayland_surface_request_resize(wl_listener* listener, void* data);

void xwayland_surface_set_geometry(wl_listener* listener, void* data);