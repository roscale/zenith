#include <epoxy/gl.h>
#include <exception>
#include "render_view.hpp"
#include "view.hpp"
#include "render_to_texture_shader.hpp"
#include "server.hpp"

extern "C" {
#define static
#include <wlr/render/gles2.h>
#undef static
}

void render_view_to_framebuffer(ZenithView* view, GLuint view_fbo) {
	glBindFramebuffer(GL_FRAMEBUFFER, view_fbo);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	RenderData rdata = {
		  .view = view,
		  .view_fbo = view_fbo,
	};

	try {
		// Render the subtree of subsurfaces starting from a toplevel or popup.
		wlr_xdg_surface_for_each_surface(
			  view->xdg_surface,
			  [](struct wlr_surface* surface, int sx, int sy, void* data) {
				  auto* rdata = static_cast<RenderData*>(data);
				  auto* view = rdata->view;

				  if (surface != view->xdg_surface->surface && wlr_surface_is_xdg_surface(surface)) {
					  // Don't render child popups. They will be rendered separately on their own framebuffer,
					  // so skip this subtree of surfaces.
					  // There's no easy way to exit this recursive iterator, but we can still do this by throwing a
					  // dummy exception.
					  throw std::exception();
				  }

				  wlr_texture* texture = wlr_surface_get_texture(surface);
				  if (texture == nullptr) {
					  // I don't remember if it's possible to have a mapped surface without texture, but better to be
					  // safe than sorry. Just skip it.
					  return;
				  }

				  wlr_gles2_texture_attribs attribs{};
				  wlr_gles2_texture_get_attribs(texture, &attribs);

				  if (wlr_surface_is_xdg_surface(surface)) {
					  // xdg_surfaces each have their own framebuffer, so they are always drawn at (0, 0).
					  RenderToTextureShader::instance()->render(attribs.tex, 0, 0,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_fbo);
				  } else {
					  // This is true when the surface is a wayland subsurface and not an xdg_surface.
					  // I decided to render subsurfaces on the framebuffer of the nearest xdg_surface parent.
					  // One downside to this approach is that popups created using a subsurface instead of an xdg_popup
					  // are clipped to the window and cannot be rendered outside the window bounds.
					  // I don't really care because Gnome Shell takes the same approach, and it simplifies things a lot.
					  // Interesting discussion: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/24

					  // Wayland surfaces only have integer scaling support. sx and sy are in the surface coordinate system,
					  // and must be scaled by the buffer scaling.
					  // https://wayland.app/protocols/wayland#wl_surface:request:set_buffer_scale
					  // https://wayland.app/protocols/wayland#wl_subsurface:request:set_position
					  int scaled_sx = sx * surface->current.scale;
					  int scaled_sy = sy * surface->current.scale;

					  RenderToTextureShader::instance()->render(attribs.tex, scaled_sx, scaled_sy,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_fbo);
				  }

				  // Tell the client we are done rendering this surface.
				  timespec now{};
				  clock_gettime(CLOCK_MONOTONIC, &now);
				  wlr_surface_send_frame_done(surface, &now);
			  },
			  &rdata
		);
	} catch (std::exception& unused) {
	}
}