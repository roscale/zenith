#pragma once

#include <mutex>

#include "embedder.h"
#include "platform_channels/binary_messenger.hpp"
#include "platform_channels/incoming_message_dispatcher.hpp"
#include "platform_channels/method_channel.h"
#include "util/render_to_texture_shader.hpp"
#include "util/framebuffer.hpp"
#include "task_runner.hpp"
#include "gl_mutex.hpp"
#include "basic_message_channel.h"
#include "document.h"

extern "C" {
#include <wlr/render/egl.h>
}

struct ZenithServer;

struct EmbedderState {
	EmbedderState(ZenithServer* server, wlr_egl* main_egl);

	void run_engine();

	void start_engine();

	void register_platform_api();

	void send_window_metrics(FlutterWindowMetricsEvent& metrics) const;

	ZenithServer* server = nullptr;
	FlutterEngine engine = nullptr;

	TaskRunner platform_task_runner{};

	/// Send messages to Dart code.
	BinaryMessenger messenger{};
	IncomingMessageDispatcher message_dispatcher;
	flutter::BasicMessageChannel<rapidjson::Document> keyEventChannel;

	wlr_egl* flutter_gl_context = nullptr;
	wlr_egl* flutter_resource_gl_context = nullptr;

	/// Allow Dart code to call C++ methods though this channel.
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;
	std::unique_ptr<flutter::MethodChannel<>> mouse_cursor_method_channel;

	intptr_t baton = 0;
	bool new_baton = false;
	std::mutex baton_mutex{};
};
