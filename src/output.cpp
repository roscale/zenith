#include <cassert>
#include "output.hpp"
#include "server.hpp"
#include "flutter_callbacks.hpp"
#include "wayland_view.hpp"

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
}

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	ZenithServer* server = output->server;
	auto& flutter_engine_state = server->flutter_engine_state;
	uint64_t now = FlutterEngineGetCurrentTime();

	wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
	wlr_egl_make_current(egl);

	for (auto& pair: server->views_by_id) {
		size_t view_id = pair.first;
		std::unique_ptr<ZenithView>& view = pair.second;

		if (!view->mapped) {
			// An unmapped view should not be rendered.
			continue;
		}

		std::shared_ptr<SurfaceFramebuffer> view_framebuffer;

		{
			std::scoped_lock lock(server->surface_framebuffers_mutex);

			auto surface_framebuffer_it = server->surface_framebuffers.find(view->id);
			assert(surface_framebuffer_it != server->surface_framebuffers.end());

			view_framebuffer = surface_framebuffer_it->second;
		}

		std::scoped_lock lock(view_framebuffer->mutex);

		GLuint view_fbo = view_framebuffer->framebuffer;

		view->render_to_fbo(view_fbo);

		FlutterEngineMarkExternalTextureFrameAvailable(flutter_engine_state->engine, (int64_t) view_id);
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
			FlutterEngineOnVsync(flutter_engine_state->engine, baton, now, now + 1'000'000'000ull / refresh_rate);
		}
	}

	/*
	 * Copy the frame to the screen.
	 */
	if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);
	wlr_renderer_begin(server->renderer, width, height);

	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(server->renderer);

	{
		std::scoped_lock lock(flutter_engine_state->present_fbo->mutex);
		GLScopedLock gl_lock(flutter_engine_state->gl_mutex);

		glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		RenderToTextureShader::instance()->render(flutter_engine_state->present_fbo->texture, 0, 0, width,
		                                          height, output_fbo);
	}

	/* Hardware cursors are rendered by the GPU on a separate plane, and can be
	 * moved around without re-rendering what's beneath them - which is more
	 * efficient. However, not all hardware supports hardware cursors. For this
	 * reason, wlroots provides a software fallback, which we ask it to render
	 * here. wlr_cursor handles configuring hardware vs software cursors for you,
	 * and this function is a no-op when hardware cursors are in use. */
	wlr_output_render_software_cursors(output->wlr_output, nullptr);

	wlr_renderer_end(server->renderer);
	wlr_output_commit(output->wlr_output);

	// I tried using a timer to apply the scrolling but on high CPU load the timer is not triggered
	// often enough. This is a good place because it forces the scroll to update on every frame.
	server->pointer->kinetic_scrolling.apply_kinetic_scrolling(server->seat);
}

void mode_changed_event(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, mode_changed);

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = width;
	window_metrics.height = height;
	window_metrics.pixel_ratio = 1.0;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));
	output->server->flutter_engine_state->send_window_metrics(window_metrics);
}