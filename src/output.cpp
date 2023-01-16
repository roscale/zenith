#include "output.hpp"
#include "server.hpp"
#include "embedder_callbacks.hpp"
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

	// TEMPORARY
//	vsync_temp_loop = wl_event_loop_add_timer(event_loop, vsync_temp_l, this);
//	wl_event_source_timer_update(vsync_temp_loop, 1000);
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

//	if (output->wlr_output->back_buffer == nullptr) {
//		wlr_output_attach_render(output->wlr_output, nullptr);
//	}
//	wlr_output_commit(output->wlr_output);

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
