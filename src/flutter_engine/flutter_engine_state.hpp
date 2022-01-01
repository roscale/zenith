#pragma once

#include <mutex>

#include "embedder.h"
#include "platform_channels/binary_messenger.hpp"
#include "platform_channels/incoming_message_dispatcher.hpp"
#include "platform_channels/method_channel.h"
#include "util/fix_y_flip.hpp"
#include "util/render_to_texture_shader.hpp"
#include "util/surface_framebuffer.hpp"
#include "task_runner.hpp"

extern "C" {
#include <wlr/render/egl.h>
}

struct ZenithOutput;

struct FlutterEngineState {
	FlutterEngineState(ZenithOutput* output, wlr_egl* main_egl);

	void run_engine();

	void start_engine();

	void register_host_api();

	ZenithOutput* output = nullptr;
	FlutterEngine engine = nullptr;

	TaskRunner platform_task_runner{};
	wl_event_source* platform_task_runner_timer{};

	/// Send messages to Dart code.
	BinaryMessenger messenger{};
	IncomingMessageDispatcher message_dispatcher;
	wlr_egl* flutter_gl_context = nullptr;
	wlr_egl* flutter_resource_gl_context = nullptr;

	/// Allow Dart code to call C++ methods though this channel.
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;

	intptr_t baton = 0;
	bool new_baton = false;
	pthread_mutex_t baton_mutex{};
	fix_y_flip_state fix_y_flip{};

	std::mutex flip_mutex{};
	std::unique_ptr<SurfaceFramebuffer> present_fbo = nullptr;
};