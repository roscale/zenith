#include <iostream>
#include "debug.hpp"
#include <epoxy/gl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION

extern "C" {
#include "stb_image_write.h"
#include <wayland-util.h>
#define static
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>
#undef static
}

void print_surface_tree_debug_info(wlr_surface* surface, int x, int y, int indents, wlr_subsurface* subsurface) {
	auto indent = [indents]() {
		for (int i = 0; i < indents; i++) {
			std::cout << " ";
		}
	};

	indent();
	std::cout << surface << std::endl;

	indent();
	std::cout << "wl_surface -> "
	          << "role = " << surface->role->name
	          << ", width = " << surface->current.width
	          << ", height = " << surface->current.height
	          << ", dx = " << surface->current.buffer_width
	          << ", dy = " << surface->current.buffer_width
	          << ", buf_width = " << surface->current.buffer_width
	          << ", buf_height = " << surface->current.buffer_height
	          << std::endl;

	if (subsurface != nullptr) {
		indent();
		std::cout << "wl_subsurface -> "
		          << "x = " << subsurface->current.x
		          << ", y = " << subsurface->current.y
		          << std::endl;
	}

	if (wlr_surface_is_xdg_surface(surface)) {
		indent();
		wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);
		std::cout << "xdg_surface -> "
		          << "geom_x = " << xdg_surface->current.geometry.x
		          << ", geom_y = " << xdg_surface->current.geometry.y
		          << ", geom_width = " << xdg_surface->current.geometry.width
		          << ", geom_height = " << xdg_surface->current.geometry.height
		          << std::endl;

		if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
			indent();
			std::cout << "xdg_toplevel -> "
			          << "title = " << xdg_surface->toplevel->title
			          << std::endl;
		}

		if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
			indent();
			std::cout << "xdg_popup -> "
			          << "geom_x = " << xdg_surface->popup->geometry.x
			          << ", geom_y = " << xdg_surface->popup->geometry.y
			          << ", geom_width = " << xdg_surface->popup->geometry.width
			          << ", geom_height = " << xdg_surface->popup->geometry.height
			          << std::endl;
		}

		if (!wl_list_empty(&xdg_surface->popups)) {
			indent();
			std::cout << "popups: " << std::endl;
		}

		struct wlr_xdg_popup* popup;
		wl_list_for_each(popup, &xdg_surface->popups, link) {
			wlr_xdg_surface* base = popup->base;
			if (!base->mapped) {
				continue;
			}

			print_surface_tree_debug_info(base->surface, 0, 0, indents + 4);
		}
	}

	if (!wl_list_empty(&surface->current.subsurfaces_below)) {
		indent();
		std::cout << "below: " << std::endl;
	}

	struct wlr_subsurface* sub;
	wl_list_for_each(sub, &surface->current.subsurfaces_below, current.link) {
		if (!sub->mapped) {
			continue;
		}

		struct wlr_subsurface_parent_state* state = &sub->current;
		int sx = state->x;
		int sy = state->y;

		print_surface_tree_debug_info(sub->surface, x + sx, y + sy, indents + 4, sub);
	}

	if (!wl_list_empty(&surface->current.subsurfaces_above)) {
		indent();
		std::cout << "above: " << std::endl;
	}

	wl_list_for_each(sub, &surface->current.subsurfaces_above, current.link) {
		if (!sub->mapped) {
			continue;
		}

		struct wlr_subsurface_parent_state* state = &sub->current;
		int sx = state->x;
		int sy = state->y;

		print_surface_tree_debug_info(sub->surface, x + sx, y + sy, indents + 4, sub);
	}
}

[[maybe_unused]] void dump_framebuffer_to_file(const char* filename, int framebuffer, int width, int height) {
	GLint framebuffer_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_binding);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	auto* buf = new unsigned char[width * height * 4];
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	stbi_write_bmp(filename, width, height, 4, buf);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	delete[] buf;
}
