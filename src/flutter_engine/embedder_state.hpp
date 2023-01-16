#pragma once

#include <mutex>

#include "embedder.h"
#include "platform_channels/binary_messenger.hpp"
#include "platform_channels/incoming_message_dispatcher.hpp"
#include "platform_channels/method_channel.h"
#include "task_runner.hpp"
#include "basic_message_channel.h"
#include "document.h"
#include "surface_buffer_chain.hpp"

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

	// Flutter needs some tasks run on the main thread. This task runner is tied to the Wayland
	// event loop and schedules tasks at the right time.
	TaskRunner platform_task_runner{};

	// Send messages to Dart code.
	BinaryMessenger messenger{};
	IncomingMessageDispatcher message_dispatcher;

	// Represents the `flutter/keyevent` channel provided by Flutter to send key events.
	// https://api.flutter.dev/flutter/services/KeyEvent-class.html
	flutter::BasicMessageChannel<rapidjson::Document> keyEventChannel;

	// Flutter needs separate OpenGL contexts to render asynchronously.
	wlr_egl* flutter_gl_context = nullptr;
	wlr_egl* flutter_resource_gl_context = nullptr;

	/// Allow Dart code to call C++ methods though this channel.
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;
	// Represents the 'flutter/mousecursor' channel provided by Flutter. When the cursor should
	// change, we receive a method call on this channel.
	std::unique_ptr<flutter::MethodChannel<>> mouse_cursor_method_channel;

	// After every vblank, Flutter gives us an opaque handle, and we need to give it back at the
	// next vblank.
	intptr_t baton = 0;
	bool new_baton = false;
	std::mutex baton_mutex{};

	// Keeps alive surface buffers, even after the surfaces are destroyed.
	// This allows Flutter to play closing animations on surfaces that don't exist anymore.
	std::unordered_map<size_t, std::shared_ptr<SurfaceBufferChain>> buffer_chains_in_use = {};
};
