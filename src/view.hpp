#pragma once

#include <cstddef>
#include <memory>

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithServer;
struct ZenithTextInput;

struct ZenithSurface {
	explicit ZenithSurface(wlr_surface* surface);

	wlr_surface* surface;
	size_t id;
	ZenithTextInput* active_text_input{};

	/* callbacks */
	wl_listener commit{};
	wl_listener new_subsurface{};
	wl_listener destroy{};
};

void surface_commit(wl_listener* listener, void* data);

void surface_destroy(wl_listener* listener, void* data);

struct ZenithXdgSurface {
	ZenithXdgSurface(wlr_xdg_surface* xdg_surface, std::shared_ptr<ZenithSurface> zenith_surface);

	wlr_xdg_surface* xdg_surface;
	std::shared_ptr<ZenithSurface> zenith_surface;

//	union {
//		ZenithXdgToplevel* toplevel;
//		ZenithXdgPopup* popup;
//	};

	/* callbacks */
	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};
};

void xdg_surface_map2(wl_listener* listener, void* data);

void xdg_surface_unmap2(wl_listener* listener, void* data);

void xdg_surface_destroy2(wl_listener* listener, void* data);

struct ZenithXdgToplevel {
	ZenithXdgToplevel(wlr_xdg_toplevel* xdg_toplevel,
	                  std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface);

	wlr_xdg_toplevel* xdg_toplevel;
	std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface;
	bool visible = true;

	/* callbacks */
	// TODO: set_title, set_app_id, etc.

	void focus() const;

	void maximize() const;
};

struct ZenithXdgPopup {
	ZenithXdgPopup(wlr_xdg_popup* xdg_popup, std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface);

	wlr_xdg_popup* xdg_popup;
	std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface;
};

struct ZenithSubsurface {
	explicit ZenithSubsurface(wlr_subsurface* subsurface, std::shared_ptr<ZenithSurface> zenith_surface);

	wlr_subsurface* subsurface;
	std::shared_ptr<ZenithSurface> zenith_surface;

	wl_listener map{};
	wl_listener unmap{};
	wl_listener destroy{};
};

void surface_new_subsurface(wl_listener* listener, void* data);

void subsurface_map(wl_listener* listener, void* data);

void subsurface_unmap(wl_listener* listener, void* data);

void subsurface_destroy(wl_listener* listener, void* data);
