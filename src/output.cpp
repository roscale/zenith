#include <sys/eventfd.h>
#include "output.hpp"
#include "server.hpp"
#include "embedder_callbacks.hpp"
#include <unistd.h>

extern "C" {
#define static
#include <wlr/render/gles2.h>
#include <wlr/util/log.h>
#include <wlr/backend/drm.h>
#undef static
}

int vsync_temp_l(void* data) {
	vsync_callback(ZenithServer::instance());
	auto* output = static_cast<ZenithOutput*>(data);
	wl_event_source_timer_update(output->vsync_temp_loop, 1000);
	return 0;
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

	// TEMPORARY
	vsync_temp_loop = wl_event_loop_add_timer(event_loop, vsync_temp_l, this);
	wl_event_source_timer_update(vsync_temp_loop, 1000);
}

static size_t i = 1;

void output_create_handle(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_output);
	auto* wlr_output = static_cast<struct wlr_output*>(data);

	/* Configures the output created by the backend to use our allocator and our renderer */
	wlr_output_init_render(wlr_output, server->allocator, server->renderer);

	static const char* selected_output_str = getenv("ZENITH_OUTPUT");
	static size_t selected_output = selected_output_str != nullptr
	                                ? selected_output_str[0] - '0'
	                                : 0;

	if (server->output != nullptr || i <= selected_output) {
		i += 1;
		// Allow only one output for the time being.
		return;
	}

	if (!wl_list_empty(&wlr_output->modes)) {
		// Set the preferred resolution and refresh rate of the monitor which will probably be the highest one.
		wlr_output_enable(wlr_output, true);
		wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
		wlr_output_set_mode(wlr_output, mode);

		if (!wlr_output_commit(wlr_output)) {
			return;
		}
	}

	// Create the output.
	auto output = std::make_unique<ZenithOutput>(server, wlr_output);
	wlr_output_layout_add_auto(server->output_layout, wlr_output);

	// Cache the layout box.
	wlr_box* box = wlr_output_layout_get_box(server->output_layout, nullptr);
	server->output_layout_box = *box;

	// Tell Flutter how big the screen is, so it can start rendering.
	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = output->wlr_output->width;
	window_metrics.height = output->wlr_output->height;
	window_metrics.pixel_ratio = server->display_scale;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(server->renderer));
	server->embedder_state->send_window_metrics(window_metrics);

	server->output = std::move(output);
}

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	ZenithServer* server = output->server;

	auto& flutter_engine_state = server->embedder_state;


//	for (auto& [id, view]: server->views) {
//		if (!view->mapped || !view->visible || wlr_surface_get_texture(view->xdg_surface->surface) == nullptr) {
//			// An unmapped view should not be rendered.
//			continue;
//		}
//
//		std::shared_ptr<Framebuffer> view_framebuffer;
//
//		{
//			std::scoped_lock lock(server->surface_framebuffers_mutex);
//
//			auto surface_framebuffer_it = server->surface_framebuffers.find(view->active_texture);
//			assert(surface_framebuffer_it != server->surface_framebuffers.end());
//
//			view_framebuffer = surface_framebuffer_it->second;
//		}
//
//		std::scoped_lock lock(view_framebuffer->mutex);
//
//		GLuint view_fbo = view_framebuffer->framebuffer;
//		render_view_to_framebuffer(view, view_fbo);
//
//		FlutterEngineMarkExternalTextureFrameAvailable(flutter_engine_state->engine, (int64_t) view->active_texture);
//	}

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

//	wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));

	std::cout << "change mode" << std::endl;

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
		if (output->wlr_output->back_buffer != nullptr || !wlr_output_attach_render(output->wlr_output, nullptr)) {
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
	eventfd_t value;
	auto* output = static_cast<ZenithOutput*>(data);
	int read = eventfd_read(output->commit_event_fd, &value);

	auto server = ZenithServer::instance();
	for (auto& [id, view]: server->xdg_toplevels) {
		wlr_xdg_surface* xdg_surface = view->xdg_toplevel->base;
		if (!xdg_surface->mapped || !view->visible) {
			// An unmapped view should not be rendered.
			continue;
		}

		timespec now{};
		clock_gettime(CLOCK_MONOTONIC, &now);
		// Notify all mapped surfaces belonging to this toplevel.
		wlr_xdg_surface_for_each_surface(xdg_surface, [](struct wlr_surface* surface, int sx, int sy, void* data) {
			auto* now = static_cast<timespec*>(data);
			wlr_surface_send_frame_done(surface, now);
		}, &now);
	}

	if (read != -1) {
		if (!wlr_output_commit(output->wlr_output)) {
			std::cerr << "commit failed\n";
			bool error = true;
			write(output->commit_event_return_pipes[1], &error, sizeof error);
			return 0;
		}
	}

//
//	for (auto* buf: server->locked_buffers) {
//		wlr_buffer_unlock(buf);
//	}
//	server->locked_buffers.clear();


	bool success = true;
	write(output->commit_event_return_pipes[1], &success, sizeof success);
	return 0;
}

#pragma clang diagnostic pop
