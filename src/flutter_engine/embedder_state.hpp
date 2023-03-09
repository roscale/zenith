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
#include "callable_queue.hpp"
#include "message_structs.hpp"

extern "C" {
#include <wlr/render/egl.h>
}

struct ZenithServer;

struct EmbedderState {
	EmbedderState(wlr_egl* flutter_gl_context, wlr_egl* flutter_resource_gl_context);

	void run_engine();

	FlutterEngine get_engine() const;

	std::optional<intptr_t> get_baton();

	void set_baton(intptr_t baton);

	void send_window_metrics(FlutterWindowMetricsEvent metrics);

	void on_vsync(intptr_t baton, uint64_t frame_start_time_nanos, uint64_t frame_target_time_nanos);

	void send_pointer_event(FlutterPointerEvent pointer_event);

	void send_key_event(const KeyboardKeyEventMessage& message);

	void register_external_texture(int64_t id);

	void mark_external_texture_frame_available(int64_t id);

	void commit_surface(const SurfaceCommitMessage& message);

	void map_xdg_surface(size_t view_id);

	void unmap_xdg_surface(size_t view_id);

	void map_subsurface(size_t view_id);

	void unmap_subsurface(size_t view_id);

	void send_text_input_event(size_t view_id, TextInputEventType event_type);

	void interactive_move(size_t view_id);

	/*
	 * The following fields have to be public because they are used by Flutter engine callbacks,
	 * which are just plain C functions.
	 */

	IncomingMessageDispatcher message_dispatcher;

	// Flutter needs separate OpenGL contexts to render asynchronously.
	wlr_egl* flutter_gl_context = nullptr;
	wlr_egl* flutter_resource_gl_context = nullptr;

	// Keeps alive surface buffers, even after the surfaces are destroyed.
	// This allows Flutter to play closing animations on surfaces that don't exist anymore.
	std::unordered_map<size_t, std::shared_ptr<SurfaceBufferChain<wlr_buffer>>> buffer_chains_in_use = {};
	std::mutex buffer_chains_mutex = {};

private:
	void configure_and_run_engine();

	void register_platform_api();

	FlutterEngine engine = nullptr;
	// The thread on which the Flutter engine and its task runner is run.
	std::thread embedder_thread;
	wl_event_loop* event_loop;
	// A function queue tied to the event loop above.
	CallableQueue callable_queue;

	// After every vblank, Flutter gives us an opaque handle, and we need to give it back at the
	// next vblank.
	intptr_t baton = 0;
	bool new_baton = false;
	std::mutex baton_mutex{};
	// Flutter needs some tasks run on the main thread. This task runner is tied to the Wayland
	// event loop and schedules tasks at the right time.
	TaskRunner platform_task_runner{};
	// Send messages to Dart code.
	BinaryMessenger messenger{};
	/// Allow Dart code to call C++ methods though this channel.
	std::unique_ptr<flutter::MethodChannel<>> platform_method_channel;
	// Represents the 'flutter/mousecursor' channel provided by Flutter. When the cursor should
	// change, we receive a method call on this channel.
	std::unique_ptr<flutter::MethodChannel<>> mouse_cursor_method_channel;
	// Represents the `flutter/keyevent` channel provided by Flutter to send key events.
	// https://api.flutter.dev/flutter/services/KeyEvent-class.html
	std::unique_ptr<flutter::BasicMessageChannel<rapidjson::Document>> key_event_channel;
};
