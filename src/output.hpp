#pragma once

#include <platform_channels/binary_messenger.hpp>
#include <platform_channels/incoming_message_dispatcher.hpp>
#include <platform_channels/method_channel.h>
#include <memory>
#include <mutex>
#include "embedder_state.hpp"

struct ZenithServer;

struct ZenithOutput {
	ZenithOutput(ZenithServer* server, struct wlr_output* wlr_output);

	ZenithServer* server = nullptr;

	struct wlr_output* wlr_output = nullptr;
	wl_listener frame_listener{};
	wl_listener mode_changed{};
};

/*
 * This function is called every time an output is ready to display a frame, generally at the output's refresh rate.
 */
void output_frame(wl_listener* listener, void* data);

void mode_changed_event(wl_listener* listener, void* data);

class OutputVBlankAddon {

};