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
	int drm_epoll_fd;
	int vsync_fd;
};

/*
 * This function is called every time an output is ready to display a frame, generally at the output's refresh rate.
 */
void output_frame(wl_listener* listener, void* data);

int drm_event(int fd, uint32_t mask, void* data);

int vblank_handler(int fd, uint32_t mask, void* data);

void page_flip_handler2(int fd,
                        unsigned int sequence,
                        unsigned int tv_sec,
                        unsigned int tv_usec,
                        unsigned int crtc_id,
                        void* user_data);

void mode_changed_event(wl_listener* listener, void* data);
