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
	wl_event_source* vsync_temp_loop;

	int attach_event_fd;
	int attach_event_return_pipes[2];
	int commit_event_fd;
	int commit_event_return_pipes[2];
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

int handle_output_attach(int fd, uint32_t mask, void* data);

int handle_output_commit(int fd, uint32_t mask, void* data);
