#include "output.hpp"
#include "server.hpp"
#include "embedder_callbacks.hpp"
#include "util/wlr/wlr_helpers.hpp"
#include "swap_chain.hpp"
#include "util/wlr/scoped_wlr_buffer.hpp"
#include "debug.hpp"
#include <unistd.h>

extern "C" {
#include <libdrm/drm_fourcc.h>
#include <GLES2/gl2ext.h>
#define static
#include <wlr/render/gles2.h>
#include <wlr/util/log.h>
#include <wlr/backend/drm.h>
#include <wlr/render/allocator.h>
#include <wlr/render/interface.h>
#undef static
}

ZenithOutput::ZenithOutput(struct wlr_output* wlr_output)
	  : wlr_output(wlr_output) {

	auto* server = ZenithServer::instance();

	frame_listener.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &frame_listener);
	mode_changed.notify = mode_changed_event;
	wl_signal_add(&wlr_output->events.mode, &mode_changed);
	destroy.notify = output_destroy;
	wl_signal_add(&wlr_output->events.destroy, &destroy);

	wlr_output_set_scale(wlr_output, server->display_scale);

	wl_event_loop* event_loop = wl_display_get_event_loop(server->display);
	schedule_frame_timer = wl_event_loop_add_timer(event_loop, [](void* data) {
		auto* output = static_cast<struct wlr_output*>(data);
		wlr_output_schedule_frame(output);
		return 0;
	}, wlr_output);
}

static size_t i = 1;

static std::unique_ptr<SwapChain<wlr_gles2_buffer>> create_swap_chain(wlr_output* wlr_output);

void output_create_handle(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_output);
	auto* wlr_output = static_cast<struct wlr_output*>(data);

	/* Configures the output created by the backend to use our allocator and our renderer */
	if (!wlr_output_init_render(wlr_output, server->allocator, server->renderer)) {
		return;
	}

	auto output = std::make_shared<ZenithOutput>(wlr_output);
	if (!output->enable()) {
		return;
	}

	// Disable the last connected output.
	if (!server->outputs.empty()) {
		const auto& last_output = server->outputs.back();
		last_output->disable();
		wlr_output_layout_remove(server->output_layout, last_output->wlr_output);
	}

	wlr_output_layout_add_auto(server->output_layout, wlr_output);

	// Tell Flutter how big the screen is, so it can start rendering.
	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = output->wlr_output->width;
	window_metrics.height = output->wlr_output->height;
	window_metrics.pixel_ratio = server->display_scale;

	server->embedder_state->send_window_metrics(window_metrics);

	server->outputs.push_back(output);
	server->output = output;
}

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	auto* server = ZenithServer::instance();
	wlr_output* wlr_output = output->wlr_output;

	vsync_callback(server);

	timespec now{};
	clock_gettime(CLOCK_MONOTONIC, &now);
	for (auto& [id, view]: server->xdg_toplevels) {
		wlr_xdg_surface* xdg_surface = view->xdg_toplevel->base;
		if (!xdg_surface->mapped || !view->visible) {
			// An unmapped view should not be rendered.
			continue;
		}

		// Notify all mapped surfaces belonging to this toplevel.
		wlr_xdg_surface_for_each_surface(xdg_surface, [](struct wlr_surface* surface, int sx, int sy, void* data) {
			auto* now = static_cast<timespec*>(data);
			wlr_surface_send_frame_done(surface, now);
		}, &now);
	}

	wlr_gles2_buffer* buffer = output->swap_chain->start_read();
	wlr_output_attach_buffer(wlr_output, buffer->buffer);
	if (!wlr_output_commit(wlr_output)) {
		// If committing fails for some reason, manually schedule a new frame, otherwise rendering stops completely.
		// After 1 ms because if we do it right away, it will saturate the event loop and no other
		// tasks will execute.
		std::cerr << "commit failed" << std::endl;
		wl_event_source_timer_update(output->schedule_frame_timer, 1);
		return;
	}
}

void mode_changed_event(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, mode_changed);
	auto* server = ZenithServer::instance();

	output->recreate_swapchain();

	if (output == ZenithServer::instance()->output.get()) {
		int width, height;
		wlr_output_effective_resolution(output->wlr_output, &width, &height);

		FlutterWindowMetricsEvent window_metrics = {};
		window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
		window_metrics.width = output->wlr_output->width;
		window_metrics.height = output->wlr_output->height;
		window_metrics.pixel_ratio = server->display_scale;

		server->embedder_state->send_window_metrics(window_metrics);
	}
}

int vsync_callback(void* data) {
	auto* server = static_cast<ZenithServer*>(data);
	auto& output = server->output;
	auto& embedder_state = server->embedder_state;

	/*
	 * Notify the compositor to prepare a new frame for the next time.
	 */
	std::optional<intptr_t> baton = embedder_state->get_baton();
	if (baton.has_value()) {
		double refresh_rate = output->wlr_output->refresh != 0
		                      ? (double) output->wlr_output->refresh / 1000
		                      : 60; // Suppose it's 60Hz if the refresh rate is not available.

		uint64_t now = FlutterEngineGetCurrentTime();
		uint64_t next_frame = now + (uint64_t) (1'000'000'000ull / refresh_rate);
		embedder_state->on_vsync(*baton, now, next_frame);
	}
	return 0;
}

std::unique_ptr<SwapChain<wlr_gles2_buffer>> create_swap_chain(wlr_output* wlr_output) {
	ZenithServer* server = ZenithServer::instance();

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(server->renderer));

	std::array<std::shared_ptr<wlr_gles2_buffer>, 4> buffers = {};

	wlr_drm_format* drm_format = get_output_format(wlr_output);
	for (auto& buffer: buffers) {
		wlr_buffer* buf = wlr_allocator_create_buffer(server->allocator, wlr_output->width, wlr_output->height,
		                                              drm_format);
		assert(wlr_renderer_is_gles2(server->renderer));
		auto* gles2_renderer = (struct wlr_gles2_renderer*) server->renderer;
		wlr_gles2_buffer* gles2_buffer = create_buffer(gles2_renderer, buf);
		buffer = scoped_wlr_gles2_buffer(gles2_buffer);
	}

	return std::make_unique<SwapChain<wlr_gles2_buffer>>(buffers);
}

void output_destroy(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, destroy);
	auto* server = ZenithServer::instance();

	wlr_output_layout_remove(server->output_layout, output->wlr_output);

	auto it = std::remove_if(server->outputs.begin(), server->outputs.end(),
	                         [output](const std::shared_ptr<ZenithOutput>& o) {
		                         return o.get() == output;
	                         });
	server->outputs.erase(it, server->outputs.end());

	if (!server->outputs.empty()) {
		server->output = server->outputs.back();
	} else {
		server->output = nullptr;
		return;
	}

	if (!server->output->enable()) {
		return;
	}

	wlr_output_layout_add_auto(server->output_layout, server->output->wlr_output);

	int width, height;
	wlr_output_effective_resolution(server->output->wlr_output, &width, &height);

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = server->output->wlr_output->width;
	window_metrics.height = server->output->wlr_output->height;
	window_metrics.pixel_ratio = server->display_scale;

	server->embedder_state->send_window_metrics(window_metrics);
}

bool ZenithOutput::enable() {
	wlr_output_enable(wlr_output, true);
	// Set the preferred resolution and refresh rate of the monitor which will probably be the highest one.
	wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
	wlr_output_set_mode(wlr_output, mode);

	if (!wlr_output_commit(wlr_output)) {
		return false;
	}

	recreate_swapchain();
	return true;
}

bool ZenithOutput::disable() const {
	wlr_output_enable(wlr_output, false);
	if (!wlr_output_commit(wlr_output)) {
		return false;
	}
	return true;
}

void ZenithOutput::recreate_swapchain() {
	swap_chain = create_swap_chain(wlr_output);
}
