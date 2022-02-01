#pragma once

#include "view.hpp"

struct ZenithWaylandView : public ZenithView {
	ZenithWaylandView(ZenithServer* server, wlr_xdg_surface* xdg_surface);

	wlr_xdg_surface* xdg_surface;

	wlr_box geometry{};
	wlr_box popup_geometry{};

	/* callbacks */
	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};
	wl_listener commit{};
	wl_listener request_move{};
	wl_listener request_resize{};

	void focus() override;

	void close() override;

	void pointer_hover(double x, double y) override;

	void resize(uint32_t width, uint32_t height) override;

	void render_to_fbo(GLuint fbo) override;
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