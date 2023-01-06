#include <cassert>
#include <sys/eventfd.h>
#include "output.hpp"
#include "server.hpp"
#include "embedder_callbacks.hpp"
#include "render_view.hpp"
#include <unistd.h>

extern "C" {
#define static
#include <wlr/render/gles2.h>
#include <wlr/util/log.h>
#include <wlr/backend/drm.h>
#undef static
}

ZenithOutput::ZenithOutput(ZenithServer* server, struct wlr_output* wlr_output)
	  : server(server), wlr_output(wlr_output) {

	frame_listener.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &frame_listener);
	mode_changed.notify = mode_changed_event;
	wl_signal_add(&wlr_output->events.mode, &mode_changed);

	wlr_output_set_scale(wlr_output, server->display_scale);

	attach_event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	pipe(attach_event_return_pipes);

	commit_event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	pipe(commit_event_return_pipes);

	wl_event_loop* event_loop = wl_display_get_event_loop(server->display);
	wl_event_loop_add_fd(event_loop, attach_event_fd, WL_EVENT_READABLE,
	                     handle_output_attach, this);
	wl_event_loop_add_fd(event_loop, commit_event_fd, WL_EVENT_READABLE,
	                     handle_output_commit, this);
}

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	ZenithServer* server = output->server;

	auto& flutter_engine_state = server->embedder_state;

	for (auto& [id, view]: server->views) {
		if (!view->mapped || !view->visible || wlr_surface_get_texture(view->xdg_surface->surface) == nullptr) {
			// An unmapped view should not be rendered.
			continue;
		}

		std::shared_ptr<Framebuffer> view_framebuffer;

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

	vsync_callback(server);
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
	window_metrics.pixel_ratio = output->server->display_scale;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));
	output->server->embedder_state->send_window_metrics(window_metrics);
}

int vsync_callback(void* data) {
	auto* server = static_cast<ZenithServer*>(data);
	auto& output = server->output;
	auto& flutter_engine_state = server->embedder_state;

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

		uint64_t now = FlutterEngineGetCurrentTime();
		uint64_t next_frame = now + (uint64_t) (1'000'000'000ull / refresh_rate);
		FlutterEngineOnVsync(flutter_engine_state->engine, baton, now, next_frame);
	}
	return 0;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"

int handle_output_attach(int fd, uint32_t mask, void* data) {
	auto* output = static_cast<ZenithOutput*>(data);
	eventfd_t value;
	if (eventfd_read(output->attach_event_fd, &value) != -1) {
		if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
			std::cerr << "attach failed\n";
			GLint error = 0;
			write(output->attach_event_return_pipes[1], &error, sizeof error);
			return 0;
		}
	}
	GLint fb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);

	write(output->attach_event_return_pipes[1], &fb, sizeof fb);
	return 0;
}

int handle_output_commit(int fd, uint32_t mask, void* data) {
	auto* output = static_cast<ZenithOutput*>(data);
	eventfd_t value;
	if (eventfd_read(output->commit_event_fd, &value) != -1) {
		if (!wlr_output_commit(output->wlr_output)) {
			std::cerr << "commit failed\n";
			bool error = true;
			write(output->commit_event_return_pipes[1], &error, sizeof error);
			return 0;
		}
	}
	bool success = true;
	write(output->commit_event_return_pipes[1], &success, sizeof success);
	return 0;
}

#pragma clang diagnostic pop
