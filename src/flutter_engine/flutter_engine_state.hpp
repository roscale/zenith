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
#include "gl_mutex.hpp"

extern "C" {
#include <wlr/render/egl.h>
}

struct ZenithServer;

struct FlutterEngineState {
	FlutterEngineState(ZenithServer* server, wlr_egl* main_egl);

	void run_engine();

	void start_engine();

	void register_host_api();

	void send_window_metrics(FlutterWindowMetricsEvent& metrics);

	ZenithServer* server = nullptr;
	FlutterEngine engine = nullptr;

	TaskRunner platform_task_runner{};
	wl_event_source* platform_task_runner_timer{};

	/// Send messages to Dart code.
	BinaryMessenger messenger{};
	IncomingMessageDispatcher message_dispatcher;
//	wlr_egl* flutter_gl_context = nullptr;
	wlr_egl* flutter_resource_gl_context = nullptr;

	/// Allow Dart code to call C++ methods though this channel.
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;

	intptr_t baton = 0;
	bool new_baton = false;
	std::mutex baton_mutex{};
	fix_y_flip_state fix_y_flip{};

	std::unique_ptr<SurfaceFramebuffer> present_fbo = nullptr;
	GLMutex gl_mutex{};
	std::list<std::shared_ptr<SurfaceFramebuffer>> framebuffers_in_use{};
};