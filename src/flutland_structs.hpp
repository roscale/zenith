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
#include <src/platform_channels/method_channel.h>

struct flutland_server {
	struct wl_display* wl_display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;
	struct wlr_xdg_shell* xdg_shell;

	struct wlr_output_layout* output_layout;
	struct flutland_output* output;

	struct wl_listener new_output;
	struct wl_listener new_xdg_surface;
	std::map<struct wlr_surface*, struct flutland_view*> views;
	struct flutland_view* active_view;

	struct wlr_cursor* cursor;
	struct wlr_xcursor_manager* cursor_mgr;
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;

	struct wlr_seat* seat;
	struct wl_listener new_input;
	struct wl_list keyboards;
};

struct flutland_output {
	struct wl_list link;
	struct flutland_server* server;
	struct wlr_output* wlr_output;
	struct wl_listener frame;

	FlutterEngine engine;
	BinaryMessenger messenger;
	IncomingMessageDispatcher message_dispatcher;
	struct wlr_egl* platform_thread_egl_context;

	std::unique_ptr<flutter::EventChannel<>> window_mapped_event_channel;
	std::unique_ptr<flutter::EventChannel<>> window_unmapped_event_channel;
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;

	bool new_baton = false;
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

struct flutland_keyboard {
	struct wl_list link;
	struct flutland_server* server;
	struct wlr_input_device* device;

	struct wl_listener modifiers;
	struct wl_listener key;
};
