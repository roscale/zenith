#pragma once

#include "embedder.h"
#include "fix_y_flip.hpp"
#include "new_texture_stream_handler.hpp"

#include <wayland-server.h>

#include <semaphore.h>
#include <src/platform_channels/binary_messenger.hpp>
#include <src/platform_channels/incoming_message_dispatcher.hpp>
#include <src/platform_channels/standard_method_codec.h>
#include <src/platform_channels/event_channel.h>

struct flutland_server {
	struct wl_display* wl_display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;
	struct wlr_xdg_shell* xdg_shell;

	struct wlr_output_layout* output_layout;
	struct flutland_output* output;

	struct wl_listener new_output;
	struct wl_listener new_xdg_surface;
	struct wl_list views;
};

struct flutland_output {
	struct wl_list link;
	struct flutland_server* server;
	struct wlr_output* wlr_output;
	struct wl_listener frame;

	FlutterEngine engine;
	BinaryMessenger messenger;
	IncomingMessageDispatcher message_dispatcher;

	std::unique_ptr<flutter::EventChannel<>> new_texture_event_channel;
	std::unique_ptr<NewTextureStreamHandler<>> new_texture_stream_handler;

	intptr_t baton;
	pthread_mutex_t baton_mutex;
	sem_t vsync_semaphore;
	struct fix_y_flip_state fix_y_flip_state;
};

struct flutland_view {
	struct wl_list link;
	struct flutland_server* server;
	struct wlr_xdg_surface* xdg_surface;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
	bool mapped;
	int x, y;
};