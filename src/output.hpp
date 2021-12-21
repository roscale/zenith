#pragma once

#include <platform_channels/binary_messenger.hpp>
#include <platform_channels/incoming_message_dispatcher.hpp>
#include <platform_channels/method_channel.h>
#include <memory>
#include <mutex>
#include "flutter_engine_state.hpp"

struct ZenithServer;

struct ZenithOutput {
	ZenithOutput(ZenithServer* server, struct wlr_output* wlr_output);

	ZenithServer* server = nullptr;

	struct wlr_output* wlr_output = nullptr;
	wl_listener frame_listener{};

	// Set later.
	std::unique_ptr<FlutterEngineState> flutter_engine_state = nullptr;
};

/*
 * This function is called every time an output is ready to display a frame, generally at the output's refresh rate.
 */
void output_frame(wl_listener* listener, void* data);
