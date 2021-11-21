#pragma once

#include <src/platform_channels/binary_messenger.hpp>
#include <src/platform_channels/incoming_message_dispatcher.hpp>
#include <src/platform_channels/standard_method_codec.h>
#include <src/platform_channels/event_channel.h>
#include <src/platform_channels/method_channel.h>
#include "embedder.h"
#include "fix_y_flip.hpp"

extern "C" {
#include <semaphore.h>
#include <wayland-server.h>
#define static
#include <wlr/backend.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#undef static
}

struct FlutlandOutput;

struct FlutlandView;

struct FlutlandServer {
	wl_display* display;
	wlr_backend* backend;
	wlr_renderer* renderer;
	wlr_xdg_shell* xdg_shell;

	wlr_output_layout* output_layout;
	FlutlandOutput* output;

	wl_listener new_output;
	wl_listener new_xdg_surface;
	std::set<FlutlandView*> views;
	FlutlandView* active_view;

	wlr_cursor* cursor;
	wlr_xcursor_manager* cursor_mgr;
	wl_listener cursor_motion;
	wl_listener cursor_motion_absolute;
	wl_listener cursor_button;
	wl_listener cursor_axis;
	wl_listener cursor_frame;

	wlr_seat* seat;
	wl_listener new_input;
	wl_list keyboards;
};

struct FlutlandOutput {
	wl_list link;
	FlutlandServer* server;
	struct wlr_output* wlr_output;
	wl_listener frame;

	FlutterEngine engine;
	/// Send messages to Dart code.
	BinaryMessenger messenger;
	IncomingMessageDispatcher message_dispatcher;

	/// Allow Dart code to call C++ methods though this channel.
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;

	intptr_t baton;
	bool new_baton = false;
	pthread_mutex_t baton_mutex;
	sem_t vsync_semaphore;
	fix_y_flip_state fix_y_flip;
};

struct FlutlandView {
	wl_list link;
	FlutlandServer* server;
	wlr_xdg_surface* xdg_surface;
	wl_listener map;
	wl_listener unmap;
	wl_listener destroy;
	bool mapped;
	int x, y;
};

struct FlutlandKeyboard {
	wl_list link;
	FlutlandServer* server;
	wlr_input_device* device;

	wl_listener modifiers;
	wl_listener key;
};
