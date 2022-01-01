#include <cassert>
#include "output.hpp"
#include "server.hpp"
#include "flutter_callbacks.hpp"

extern "C" {
#define static
#include <wlr/render/gles2.h>
#undef static
}

ZenithOutput::ZenithOutput(ZenithServer* server, struct wlr_output* wlr_output)
	  : server(server), wlr_output(wlr_output) {

	frame_listener.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &frame_listener);
}

struct render_data {
	ZenithOutput* output;
	ZenithView* view;
	GLuint view_framebuffer;
	bool skip_surface;
};

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	ZenithServer* server = output->server;
	auto& flutter_engine_state = output->flutter_engine_state;

	wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
	wlr_egl_make_current(egl);

	for (auto& pair: server->views_by_id) {
		size_t view_id = pair.first;
		std::unique_ptr<ZenithView>& view = pair.second;

		if (!view->mapped || wlr_surface_get_texture(view->xdg_surface->surface) == nullptr) {
			/* An unmapped view should not be rendered. */
			continue;
		}

		server->surface_framebuffers_mutex.lock();

		GLuint view_framebuffer;
		auto surface_framebuffer_it = server->surface_framebuffers.find(view->id);
		if (surface_framebuffer_it == server->surface_framebuffers.end()) {
			wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);
			assert(texture != nullptr);

			auto inserted_pair = server->surface_framebuffers.insert(
				  std::pair<size_t, std::unique_ptr<SurfaceFramebuffer>>(
						view->id,
						std::make_unique<SurfaceFramebuffer>(texture->width, texture->height)
				  )
			);
			surface_framebuffer_it = inserted_pair.first;
		}
		view_framebuffer = surface_framebuffer_it->second->framebuffer;

		server->surface_framebuffers_mutex.unlock();

		glClearColor(0, 0, 0, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, view_framebuffer);
		glClear(GL_COLOR_BUFFER_BIT);

		render_data rdata = {
			  .output = output,
			  .view = view.get(),
			  .view_framebuffer = view_framebuffer,
			  .skip_surface = false,
		};

		// Render the subtree of subsurfaces starting from a toplevel or popup.
		wlr_xdg_surface_for_each_surface(
			  view->xdg_surface,
			  [](struct wlr_surface* surface, int sx, int sy, void* data) {
				  auto* rdata = static_cast<render_data*>(data);
				  auto* view = rdata->view;

				  if (rdata->skip_surface) {
					  return;
				  }

				  if (surface != view->xdg_surface->surface && wlr_surface_is_xdg_surface(surface)) {
					  // The tree of surfaces might also contain other xdg_surfaces, most notably popups.
					  // Once such surface is encountered, skip it and its surfaces because we are going to draw them
					  // in a separate pass on their own framebuffer. There's no easy way to just break out of this recursive iterator, so we'll
					  // do it using this boolean.
					  rdata->skip_surface = true;
					  return;
				  }

				  wlr_texture* texture = wlr_surface_get_texture(surface);
				  if (texture == nullptr) {
					  return;
				  }

				  wlr_gles2_texture_attribs attribs{};
				  wlr_gles2_texture_get_attribs(texture, &attribs);

				  if (wlr_surface_is_xdg_surface(surface)) {
					  // xdg_surfaces each have their own framebuffer, so they are always drawn at (0, 0) in this framebuffer.
					  RenderToTextureShader::instance()->render(attribs.tex, 0, 0,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_framebuffer);
				  } else {
					  // This is true when the surface is a wayland subsurface and not an xdg_surface.
					  // I decided to render subsurfaces on the framebuffer of the nearest xdg_surface parent.
					  // One downside to this approach is that popups created using a subsurface instead of an xdg_popup
					  // are clipped to the window and cannot be rendered outside the window bounds.
					  // I don't really care because Gnome Shell takes the same approach on Ubuntu, and it simplifies things a lot.
					  // Interesting discussion: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/24
					  RenderToTextureShader::instance()->render(attribs.tex, sx, sy,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_framebuffer);
				  }

				  // Tell the client we are done rendering this surface.
				  timespec now{};
				  clock_gettime(CLOCK_MONOTONIC, &now);
				  wlr_surface_send_frame_done(surface, &now);
			  },
			  &rdata
		);

		FlutterEngineMarkExternalTextureFrameAvailable(flutter_engine_state->engine, (int64_t) view_id);
	}

	intptr_t baton;
	pthread_mutex_lock(&flutter_engine_state->baton_mutex);
	if (flutter_engine_state->new_baton) {
		baton = flutter_engine_state->baton;
		flutter_engine_state->new_baton = false;

		uint64_t start = FlutterEngineGetCurrentTime();
		FlutterEngineOnVsync(flutter_engine_state->engine, baton, start, start + 1'000'000'000ull / 144);
	}
	pthread_mutex_unlock(&flutter_engine_state->baton_mutex);

	if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	wlr_renderer_begin(server->renderer, width, height);

	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(server->renderer);

	flutter_engine_state->flip_mutex.lock();
	glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	RenderToTextureShader::instance()->render(flutter_engine_state->present_fbo->texture, 0, 0, width,
	                                          height, output_fbo);
	flutter_engine_state->flip_mutex.unlock();

	/* Hardware cursors are rendered by the GPU on a separate plane, and can be
	 * moved around without re-rendering what's beneath them - which is more
	 * efficient. However, not all hardware supports hardware cursors. For this
	 * reason, wlroots provides a software fallback, which we ask it to render
	 * here. wlr_cursor handles configuring hardware vs software cursors for you,
	 * and this function is a no-op when hardware cursors are in use. */
	wlr_output_render_software_cursors(output->wlr_output, nullptr);

	wlr_renderer_end(server->renderer);
	wlr_output_commit(output->wlr_output);
}
