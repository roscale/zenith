#pragma once

#include <cstddef>

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithServer;
struct ZenithTextInput;

struct ZenithSurface {
	explicit ZenithSurface(wlr_surface* surface);

	wlr_surface* surface;
	size_t id;
	ZenithTextInput* active_text_input;

	/* callbacks */
	wl_listener commit{};
	wl_listener destroy{};
};

struct ZenithXdgSurface;

struct ZenithXdgToplevel {
	explicit ZenithXdgToplevel(wlr_xdg_toplevel* xdg_toplevel);

	wlr_xdg_toplevel* xdg_toplevel;
	bool visible = true;

	/* callbacks */
	// TODO

	[[nodiscard]] ZenithXdgSurface* zenith_xdg_surface() const;

	void focus() const;

	void maximize() const;
};

struct ZenithXdgPopup {
	explicit ZenithXdgPopup(wlr_xdg_popup* xdg_popup);

	wlr_xdg_popup* xdg_popup;

	[[nodiscard]] ZenithXdgSurface* zenith_xdg_surface() const;
};

struct ZenithXdgSurface {
	explicit ZenithXdgSurface(wlr_xdg_surface* xdg_surface);

	wlr_xdg_surface* xdg_surface;

	union {
		ZenithXdgToplevel toplevel;
		ZenithXdgPopup popup;
	};

	/* callbacks */
	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};

	[[nodiscard]] ZenithSurface* zenith_surface() const;
};

//struct ZenithView {
//	ZenithView(ZenithServer* server, wlr_xdg_surface* xdg_surface);
//
//	/*
//	 * Activate the view and make the keyboard enter the surface of this view.
//	 */
//	void focus() const;
//
//	void maximize() const;
//
//	ZenithServer* server;
//	wlr_xdg_surface* xdg_surface;
//	size_t id;
//	bool mapped = false;
//	bool visible = true;
//	wlr_box popup_geometry{};
//	wlr_box visible_bounds{};
//	ZenithTextInput* active_text_input = nullptr;
//	size_t active_texture = 0;
//
//	/* callbacks */
//	wl_listener map{};
//	wl_listener unmap{};
//	wl_listener destroy{};
//	wl_listener commit{};
//};

/*
 * Called when the surface is mapped, or ready to display on-screen.
 */
//void xdg_surface_map(wl_listener* listener, void* data);
//
///*
// * Called when the surface is unmapped, and should no longer be shown.
// */
//void xdg_surface_unmap(wl_listener* listener, void* data);
//
///*
// * Called when the surface is destroyed and should never be shown again.
// */
//void xdg_surface_destroy(wl_listener* listener, void* data);

void surface_commit(wl_listener* listener, void* data);

void surface_destroy(wl_listener* listener, void* data);

void xdg_surface_map2(wl_listener* listener, void* data);

void xdg_surface_unmap2(wl_listener* listener, void* data);

void xdg_surface_destroy2(wl_listener* listener, void* data);
