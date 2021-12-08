#pragma once

#include <src/platform_channels/binary_messenger.hpp>
#include <src/platform_channels/incoming_message_dispatcher.hpp>
#include <src/platform_channels/standard_method_codec.h>
#include <src/platform_channels/event_channel.h>
#include <src/platform_channels/method_channel.h>
#include "embedder.h"
#include "fix_y_flip.hpp"
#include "surface_framebuffer.hpp"
#include "render_to_texture_shader.hpp"
#include <mutex>

extern "C" {
#include <semaphore.h>
#include <wayland-server.h>
#define static
#include <wlr/backend.h>
#include <wlr/render/egl.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#undef static
}

struct ZenithOutput;

struct ZenithView;

struct ZenithServer {
	wl_display* display;
	wlr_backend* backend;
	wlr_renderer* renderer;
	wlr_xdg_shell* xdg_shell;

	wlr_output_layout* output_layout;
	ZenithOutput* output;

	wl_listener new_output;
	wl_listener new_xdg_surface;
	std::map<size_t, ZenithView*> views;
	std::mutex surface_framebuffers_mutex;
	std::map<size_t, std::unique_ptr<SurfaceFramebuffer>> surface_framebuffers;

	wlr_cursor* cursor;
	wlr_xcursor_manager* cursor_mgr;
	wl_listener cursor_motion;
	wl_listener cursor_motion_absolute;
	wl_listener cursor_button;
	wl_listener cursor_axis;
	wl_listener cursor_frame;
	wl_listener request_cursor;

	wlr_seat* seat;
	wl_listener new_input;
	wl_list keyboards;
};

struct ZenithOutput {
	ZenithOutput(BinaryMessenger messenger, IncomingMessageDispatcher messageDispatcher)
		  : message_dispatcher(messageDispatcher), messenger(messenger) {

	}

	wl_list link;
	ZenithServer* server;
	struct wlr_output* wlr_output;
	wl_listener frame_listener;

	FlutterEngine engine;
	/// Send messages to Dart code.
	BinaryMessenger messenger;
	IncomingMessageDispatcher message_dispatcher;
	wlr_egl* flutter_gl_context;
	wlr_egl* flutter_resource_gl_context;

	/// Allow Dart code to call C++ methods though this channel.
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;

	intptr_t baton;
	bool new_baton = false;
	pthread_mutex_t baton_mutex;
	sem_t vsync_semaphore;
	fix_y_flip_state fix_y_flip;

	std::unique_ptr<RenderToTextureShader> render_to_texture_shader;
	std::mutex flip_mutex;
	std::unique_ptr<SurfaceFramebuffer> present_fbo;
};

struct ZenithView {
	wl_list link;
	size_t id;
	ZenithServer* server;
	wlr_xdg_surface* xdg_surface;
	wl_listener map;
	wl_listener unmap;
	wl_listener destroy;
	bool mapped;
	int x, y;
};

struct ZenithKeyboard {
	wl_list link;
	ZenithServer* server;
	wlr_input_device* device;

	wl_listener modifiers;
	wl_listener key;
};
