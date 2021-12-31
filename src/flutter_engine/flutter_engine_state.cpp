#include "flutter_engine_state.hpp"
#include "util/create_shared_egl_context.hpp"
#include "flutter_callbacks.hpp"
#include "host_api.hpp"
#include "standard_method_codec.h"
#include <filesystem>
#include <thread>
#include "server.hpp"

extern "C" {
#define static
#include <wlr/types/wlr_output.h>
#undef static
}

struct ZenithServer;

FlutterEngineState::FlutterEngineState(ZenithOutput* output, wlr_egl* main_egl)
	  : output(output), message_dispatcher(&messenger) {

	wlr_egl_context saved_egl_context{};
	wlr_egl_save_context(&saved_egl_context);

	// Create 2 OpenGL shared contexts for rendering operations.
	wlr_egl_make_current(main_egl);
	flutter_gl_context = create_shared_egl_context(main_egl);
	flutter_resource_gl_context = create_shared_egl_context(main_egl);
	wlr_egl_unset_current(main_egl);

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	// FBOs cannot be shared between GL contexts. Since these FBOs will be used by a Flutter thread, create these
	// resources using Flutter's context.
	wlr_egl_make_current(flutter_gl_context);
	fix_y_flip = fix_y_flip_init_state(width, height);
	present_fbo = std::make_unique<SurfaceFramebuffer>(width, height);
	wlr_egl_unset_current(flutter_gl_context);

	wlr_egl_restore_context(&saved_egl_context);

	pthread_mutex_init(&baton_mutex, nullptr);

	messenger.SetMessageDispatcher(&message_dispatcher);
}

void FlutterEngineState::run_engine() {
	/*
	 * Configure callbacks, app & assets path, run the engine.
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
		auto* flutter_engine_state = static_cast<FlutterEngineState*>(userdata);
		return std::this_thread::get_id() == flutter_engine_state->output->server->main_thread_id;
	};
	platform_task_runner_description.post_task_callback = [](FlutterTask task, uint64_t target_time, void* userdata) {
		auto* flutter_engine_state = static_cast<FlutterEngineState*>(userdata);
		flutter_engine_state->platform_task_runner.add_task(target_time, task);
	};

	FlutterCustomTaskRunners task_runners{};
	task_runners.struct_size = sizeof(FlutterCustomTaskRunners);
	task_runners.platform_task_runner = &platform_task_runner_description;

	auto executable_path = std::filesystem::canonical("/proc/self/exe");
	std::array command_line_argv = {
		  executable_path.c_str(),
		  "--observatory-port=12345",
		  "--disable-service-auth-codes",
	};

#ifdef DEBUG
	FlutterProjectArgs args = {};
	args.struct_size = sizeof(FlutterProjectArgs);
	args.assets_path = "data/flutter_assets";
	args.icu_data_path = "data/icudtl.dat";
	args.platform_message_callback = flutter_platform_message_callback;
	args.vsync_callback = flutter_vsync_callback;
	args.custom_task_runners = &task_runners;
	args.command_line_argc = command_line_argv.size();
	args.command_line_argv = command_line_argv.data();

#elif defined(PROFILE)
	auto absolute_path = std::filesystem::canonical("lib/libapp.so");

	FlutterEngineAOTDataSource data_source = {};
	data_source.type = kFlutterEngineAOTDataSourceTypeElfPath;
	data_source.elf_path = absolute_path.c_str();

	FlutterEngineAOTData aot_data;
	FlutterEngineCreateAOTData(&data_source, &aot_data);

	FlutterProjectArgs args = {};
	args.struct_size = sizeof(FlutterProjectArgs);
	std::cout << executable_path << std::endl;
	args.assets_path = "data/flutter_assets";
	args.icu_data_path = "data/icudtl.dat";
	args.platform_message_callback = flutter_platform_message_callback;
	args.vsync_callback = flutter_vsync_callback;
	args.aot_data = aot_data;
	args.custom_task_runners = &task_runners;
	args.command_line_argc = command_line_argv.size();
	args.command_line_argv = command_line_argv.data();

#else
	/*
	 * Release mode has to run in AOT mode and the setup is a bit different.
	 */
	auto absolute_path = std::filesystem::canonical("lib/libapp.so");

	FlutterEngineAOTDataSource data_source = {};
	data_source.type = kFlutterEngineAOTDataSourceTypeElfPath;
	data_source.elf_path = absolute_path.c_str();

	FlutterEngineAOTData aot_data;
	FlutterEngineCreateAOTData(&data_source, &aot_data);

	FlutterProjectArgs args = {};
	args.struct_size = sizeof(FlutterProjectArgs);
	args.assets_path = "data/flutter_assets";
	args.icu_data_path = "data/icudtl.dat";
	args.platform_message_callback = flutter_platform_message_callback;
	args.vsync_callback = flutter_vsync_callback;
	args.aot_data = aot_data;
	args.custom_task_runners = &task_runners;
	args.command_line_argc = command_line_argv.size();
	args.command_line_argv = command_line_argv.data();

#endif

	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, this, &engine);
	assert(result == kSuccess && engine != nullptr);

	platform_task_runner.set_engine(engine);

	/*
	 * Set up a communication channel between the Flutter app and the host.
	 */
	auto& codec = flutter::StandardMethodCodec::GetInstance();

	messenger.SetEngine(engine);
	platform_method_channel = std::make_unique<flutter::MethodChannel<>>(&messenger, "platform", &codec);

	ZenithOutput* output = this->output;
	platform_method_channel->SetMethodCallHandler(
		  [output](const flutter::MethodCall<>& call, std::unique_ptr<flutter::MethodResult<>> result) {
			  if (call.method_name() == "activate_window") {
				  activate_window(output, call, std::move(result));
			  } else if (call.method_name() == "pointer_hover") {
				  pointer_hover(output, call, std::move(result));
			  } else if (call.method_name() == "pointer_exit") {
				  pointer_exit(output, call, std::move(result));
			  } else if (call.method_name() == "close_window") {
				  close_window(output, call, std::move(result));
			  } else if (call.method_name() == "resize_window") {
				  resize_window(output, call, std::move(result));
			  } else {
				  result->Error("method_does_not_exist", "Method " + call.method_name() + " does not exist");
			  }
		  }
	);

	/*
	 * Tell Flutter how big the screen is, so it can start rendering.
	 */
	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = width;
	window_metrics.height = height;
	window_metrics.pixel_ratio = 1.0;

	FlutterEngineSendWindowMetricsEvent(engine, &window_metrics);
}
