#include <cassert>
#include "output.hpp"
#include "server.hpp"
#include "embedder_callbacks.hpp"
#include "time.hpp"
#include "render_view.hpp"

extern "C" {
#define static
#include <wlr/render/gles2.h>
#undef static
}

ZenithOutput::ZenithOutput(ZenithServer* server, struct wlr_output* wlr_output)
	  : server(server), wlr_output(wlr_output) {

	frame_listener.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &frame_listener);
	mode_changed.notify = mode_changed_event;
	wl_signal_add(&wlr_output->events.mode, &mode_changed);

	wlr_output_set_scale(wlr_output, 3.0f);
}

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	ZenithServer* server = output->server;
	auto& flutter_engine_state = server->embedder_state;

	uint64_t now = current_time_nanoseconds();

	wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
	wlr_egl_make_current(egl);

	for (auto& [id, view]: server->views) {
		if (!view->mapped || !view->visible || wlr_surface_get_texture(view->xdg_surface->surface) == nullptr) {
			// An unmapped view should not be rendered.
			continue;
		}

		std::shared_ptr <Framebuffer> view_framebuffer;

		{
			std::scoped_lock lock(server->surface_framebuffers_mutex);

			auto surface_framebuffer_it = server->surface_framebuffers.find(view->active_texture);
			assert(surface_framebuffer_it != server->surface_framebuffers.end());

			view_framebuffer = surface_framebuffer_it->second;
		}

		std::scoped_lock lock(view_framebuffer->mutex);

		GLuint view_fbo = view_framebuffer->framebuffer;
		render_view_to_framebuffer(view, view_fbo);

		FlutterEngineMarkExternalTextureFrameAvailable(flutter_engine_state->engine, (int64_t) view->active_texture);
	}

	{
		/*
		 * Notify the compositor to prepare a new frame for the next time.
		 */
		std::scoped_lock lock(flutter_engine_state->baton_mutex);

		if (flutter_engine_state->new_baton) {
			intptr_t baton = flutter_engine_state->baton;
			flutter_engine_state->new_baton = false;

			double refresh_rate = output->wlr_output->refresh != 0
			                      ? (double) output->wlr_output->refresh / 1000
			                      : 60; // Suppose it's 60Hz if the refresh rate is not available.

			uint64_t next_frame = now + (uint64_t)(1'000'000'000ull / refresh_rate);
			FlutterEngineOnVsync(flutter_engine_state->engine, baton, now, next_frame);
		}
	}

	/*
	 * Copy the frame to the screen.
	 */
	if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
		return;
	}

	int width = output->wlr_output->width;
	int height = output->wlr_output->height;
	wlr_renderer_begin(server->renderer, width, height);

	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(server->renderer);

	{
		std::scoped_lock lock(flutter_engine_state->copy_framebuffer->mutex);
		GLScopedLock gl_lock(flutter_engine_state->output_gl_mutex);

		glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		RenderToTextureShader::instance()->render(flutter_engine_state->copy_framebuffer->texture, 0, 0, width,
		                                          height, output_fbo, true);
	}

	/* Hardware cursors are rendered by the GPU on a separate plane, and can be
	 * moved around without re-rendering what's beneath them - which is more
	 * efficient. However, not all hardware supports hardware cursors. For this
	 * reason, wlroots provides a software fallback, which we ask it to render
	 * here. wlr_cursor handles configuring hardware vs software cursors for you,
	 * and this function is a no-op when hardware cursors are in use. */
	wlr_output_render_software_cursors(output->wlr_output, nullptr);

	wlr_renderer_end(server->renderer);
	if (!wlr_output_commit(output->wlr_output)) {
		wlr_output_schedule_frame(output->wlr_output);
	}
}

void mode_changed_event(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, mode_changed);

	// Update the layout box when the mode changes.
	wlr_box* box = wlr_output_layout_get_box(output->server->output_layout, nullptr);
	output->server->output_layout_box = *box;

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = output->wlr_output->width;
	window_metrics.height = output->wlr_output->height;
	window_metrics.pixel_ratio = 3.0;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));
	output->server->embedder_state->send_window_metrics(window_metrics);
}
