#include "embedder_state.hpp"
#include "util/create_shared_egl_context.hpp"
#include "embedder_callbacks.hpp"
#include "platform_api.hpp"
#include "standard_method_codec.h"
#include <filesystem>
#include <thread>
#include "server.hpp"
#include "egl_helpers.hpp"

extern "C" {
#define static
#include <wlr/types/wlr_output.h>
#undef static
}

struct ZenithServer;

EmbedderState::EmbedderState(ZenithServer* server, wlr_egl* main_egl)
	  : server(server), message_dispatcher(&messenger) {

	messenger.SetMessageDispatcher(&message_dispatcher);

	ZenithEglContext saved_egl_context{};
	zenith_egl_save_context(&saved_egl_context);

	// Create 2 OpenGL shared contexts for rendering operations.
	wlr_egl_make_current(main_egl);
	flutter_gl_context = create_shared_egl_context(main_egl);
	flutter_resource_gl_context = create_shared_egl_context(main_egl);
	wlr_egl_unset_current(main_egl);

	// Flutter doesn't know the output dimensions, so initialize all required textures with the smallest
	// size possible. When an output (screen) becomes available, the Flutter engine will be notified and
	// all textures will be resized.
	size_t dummy_width = 1;
	size_t dummy_height = 1;

	// FBOs cannot be shared between GL contexts. Since these FBOs will be used by a Flutter thread, create these
	// resources using Flutter's context.
	wlr_egl_make_current(flutter_gl_context);
	output_framebuffer = std::make_unique<Framebuffer>(dummy_width, dummy_height);
	wlr_egl_unset_current(flutter_gl_context);

	zenith_egl_restore_context(&saved_egl_context);
}

void EmbedderState::run_engine() {
	start_engine();

	platform_task_runner.set_engine(engine);
	platform_task_runner_timer = wl_event_loop_add_timer(wl_display_get_event_loop(server->display),
	                                                     flutter_execute_expired_tasks_timer, this);
	// Arm the timer.
	wl_event_source_timer_update(platform_task_runner_timer, 1);

	register_platform_api();
}

void EmbedderState::start_engine() {
	/*
	 * Configure renderer, task runners, and project args.
	 */
	FlutterRendererConfig config = {};
	config.type = kOpenGL;
	config.open_gl.struct_size = sizeof(config.open_gl);
	config.open_gl.make_current = flutter_make_current;
	config.open_gl.clear_current = flutter_clear_current;
	config.open_gl.present = flutter_present;
	config.open_gl.fbo_callback = flutter_fbo_callback;
	config.open_gl.gl_external_texture_frame_callback = flutter_gl_external_texture_frame_callback;
	config.open_gl.make_resource_current = flutter_make_resource_current;

	FlutterTaskRunnerDescription platform_task_runner_description{};
	platform_task_runner_description.struct_size = sizeof(FlutterTaskRunnerDescription);
	platform_task_runner_description.user_data = this;
	platform_task_runner_description.identifier = 1;
	platform_task_runner_description.runs_task_on_current_thread_callback = [](void* userdata) {
		auto* flutter_engine_state = static_cast<EmbedderState*>(userdata);
		return std::this_thread::get_id() == flutter_engine_state->server->main_thread_id;
	};
	platform_task_runner_description.post_task_callback = [](FlutterTask task, uint64_t target_time, void* userdata) {
		auto* flutter_engine_state = static_cast<EmbedderState*>(userdata);
		flutter_engine_state->platform_task_runner.add_task(target_time, task);
	};

	FlutterCustomTaskRunners task_runners{};
	task_runners.struct_size = sizeof(FlutterCustomTaskRunners);
	task_runners.platform_task_runner = &platform_task_runner_description;

	std::filesystem::path executable_path = std::filesystem::canonical("/proc/self/exe");
	// Normally, the observatory is started on a random port with some random auth code, but we will
	// set up a fixed URL in order to automate attaching a debugger which requires observatory's URL.
	std::array command_line_argv = {
		  executable_path.c_str(),
		  "--observatory-port=12345",
		  "--disable-service-auth-codes",
	};

	std::filesystem::path executable_directory = executable_path.parent_path();

	FlutterProjectArgs args = {};
	args.struct_size = sizeof(FlutterProjectArgs);
	std::filesystem::path assets_path = executable_directory / "data" / "flutter_assets";
	args.assets_path = assets_path.c_str();
	std::filesystem::path icu_data_path = executable_directory / "data" / "icudtl.dat";
	args.icu_data_path = icu_data_path.c_str();
	args.platform_message_callback = flutter_platform_message_callback;
	args.vsync_callback = flutter_vsync_callback;
	args.custom_task_runners = &task_runners;
	args.command_line_argc = command_line_argv.size();
	args.command_line_argv = command_line_argv.data();

#ifndef DEBUG
	/*
	 * Profile and release modes have to run in AOT mode.
	 */
	FlutterEngineAOTDataSource data_source = {};
	data_source.type = kFlutterEngineAOTDataSourceTypeElfPath;
	std::filesystem::path elf_path = executable_directory / "lib" / "libapp.so";
	data_source.elf_path = elf_path.c_str();

	FlutterEngineAOTData aot_data;
	FlutterEngineCreateAOTData(&data_source, &aot_data);

	args.aot_data = aot_data;
#endif

	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, this, &engine);
	assert(result == kSuccess && engine != nullptr);
}

void EmbedderState::register_platform_api() {
	auto& codec = flutter::StandardMethodCodec::GetInstance();

	messenger.SetEngine(engine);
	platform_method_channel = std::make_unique<flutter::MethodChannel<>>(&messenger, "platform", &codec);

	ZenithServer* server = this->server;
	platform_method_channel->SetMethodCallHandler(
		  [server](const flutter::MethodCall<>& call, std::unique_ptr<flutter::MethodResult<>> result) {
			  const std::string& method_name = call.method_name();
			  if (method_name == "startup_complete") {
				  startup_complete(server, call, std::move(result));
			  } else if (method_name == "activate_window") {
				  activate_window(server, call, std::move(result));
			  } else if (method_name == "pointer_hover") {
				  pointer_hover(server, call, std::move(result));
			  } else if (method_name == "pointer_exit") {
				  pointer_exit(server, call, std::move(result));
			  } else if (method_name == "close_window") {
				  close_window(server, call, std::move(result));
			  } else if (method_name == "resize_window") {
				  resize_window(server, call, std::move(result));
			  } else if (method_name == "unregister_view_texture") {
				  unregister_view_texture(server, call, std::move(result));
			  } else if (method_name == "mouse_button_event") {
				  mouse_button_event(server, call, std::move(result));
			  } else if (method_name == "change_window_visibility") {
				  change_window_visibility(server, call, std::move(result));
			  } else if (method_name == "touch_down") {
				  touch_down(server, call, std::move(result));
			  } else if (method_name == "touch_motion") {
				  touch_motion(server, call, std::move(result));
			  } else if (method_name == "touch_up") {
				  touch_up(server, call, std::move(result));
			  } else if (method_name == "insert_text") {
				  insert_text(server, call, std::move(result));
			  } else if (method_name == "emulate_keycode") {
				  emulate_keycode(server, call, std::move(result));
			  } else if (method_name == "initial_window_size") {
				  initial_window_size(server, call, std::move(result));
			  } else {
				  result->Error("method_does_not_exist", "Method " + method_name + " does not exist");
			  }
		  }
	);
}

void EmbedderState::send_window_metrics(FlutterWindowMetricsEvent& metrics) const {
	std::scoped_lock lock(server->embedder_state->output_framebuffer->mutex);
	GLScopedLock gl_lock(server->embedder_state->output_gl_mutex);

	FlutterEngineSendWindowMetricsEvent(server->embedder_state->engine, &metrics);

	output_framebuffer->resize(metrics.width, metrics.height);
}
