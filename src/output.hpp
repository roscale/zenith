#pragma once

#include <platform_channels/binary_messenger.hpp>
#include <platform_channels/incoming_message_dispatcher.hpp>
#include <platform_channels/method_channel.h>
#include <memory>
#include <mutex>
#include "embedder_state.hpp"
#include "swap_chain.hpp"
#include "util/wlr/wlr_helpers.hpp"
#include <GLES2/gl2.h>

extern "C" {
#define static
#include <wlr/util/addon.h>
#undef static
}

struct ZenithServer;

struct ZenithOutput {
	explicit ZenithOutput(struct wlr_output* wlr_output);

	struct wlr_output* wlr_output = nullptr;
	wl_listener frame_listener{};
	wl_listener mode_changed{};
	wl_listener destroy{};
	wl_event_source* schedule_frame_timer;

	std::unique_ptr<SwapChain<wlr_gles2_buffer>> swap_chain;

	bool enable();

	bool disable() const;

	void recreate_swapchain();
};

/*
 * This event is raised when a new output is detected, like a monitor or a projector.
 */
void output_create_handle(wl_listener* listener, void* data);

/*
 * This function is called every time an output is ready to display a frame, generally at the output's refresh rate.
 */
void output_frame(wl_listener* listener, void* data);

void mode_changed_event(wl_listener* listener, void* data);

int vsync_callback(void* data);

void output_destroy(wl_listener* listener, void* data);
