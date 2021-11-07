#include "flutter_callbacks.hpp"
#include "flutland_structs.hpp"

extern "C" {
#define static
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xcursor_manager.h>
#undef static
}

#include <GL/gl.h>
#include <cassert>
#include <iostream>

#define BUNDLE "build/linux/x64/debug/bundle/data"

FlutterEngine run_flutter(flutland_output* output) {
	FlutterRendererConfig config = {};
	config.type = kOpenGL;
	config.open_gl.struct_size = sizeof(config.open_gl);
	config.open_gl.make_current = flutter_make_current;
	config.open_gl.clear_current = flutter_clear_current;
	config.open_gl.present = flutter_present;
	config.open_gl.fbo_callback = flutter_fbo_callback;
	config.open_gl.gl_external_texture_frame_callback = flutter_gl_external_texture_frame_callback;

//	FlutterTaskRunnerDescription description = {
//		  .struct_size = sizeof(FlutterTaskRunnerDescription),
//		  .identifier = 1,
//		  .runs_task_on_current_thread_callback
//	};
//
//	FlutterCustomTaskRunners customTaskRunners = {
//		  .struct_size = sizeof(FlutterCustomTaskRunners),
//		  .platform_task_runner =
//	};

//	FlutterEngineAOTDataSource data_source = {
//			.type = kFlutterEngineAOTDataSourceTypeElfPath,
//			.elf_path = "/home/roscale/CLionProjects/flutter_embedder/build/linux/x64/release/bundle/lib/libapp.so",
//	};
//	FlutterEngineAOTData data;
//	FlutterEngineCreateAOTData(&data_source, &data);

	FlutterProjectArgs args = {
			.struct_size = sizeof(FlutterProjectArgs),
			.assets_path = BUNDLE "/flutter_assets",
			.icu_data_path = BUNDLE "/icudtl.dat",
			.platform_message_callback = flutter_platform_message_callback,
			.vsync_callback = vsync_callback,
//			.aot_data = data,
	};

	FlutterEngine engine = nullptr;
	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, output, &engine);
	assert(result == kSuccess && engine != nullptr);

	return engine;
}

bool flutter_make_current(void* userdata) {
//	std::clog << "MAKE_CURRENT" << std::endl;

	auto* output = static_cast<flutland_output*>(userdata);

	return wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));
//	return wlr_egl_make_current(output->platform_thread_egl_context);
}

bool flutter_clear_current(void* userdata) {
//	std::clog << "CLEAR_CURRENT" << std::endl;
	auto* output = static_cast<flutland_output*>(userdata);

	return wlr_egl_unset_current(wlr_gles2_renderer_get_egl(output->server->renderer));
//	return wlr_egl_unset_current(output->platform_thread_egl_context);
}

bool flutter_present(void* userdata) {
//	std::clog << "PRESENT\n" << std::endl;

	auto* output = static_cast<flutland_output*>(userdata);
	struct wlr_renderer* renderer = output->server->renderer;

	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(output->server->renderer);

//	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT);

	render_to_fbo(&output->fix_y_flip_state, output_fbo);

//	wlr_output_lock_software_cursors(output->wlr_output, true);
//	wlr_output_render_software_cursors(output->wlr_output, nullptr);

	wlr_renderer_end(renderer);

//	wlr_xcursor_manager_set_cursor_image(
//		  output->server->cursor_mgr, "left_ptr", output->server->cursor);


	wlr_output_commit(output->wlr_output);

//	GLint texture_binding;
//	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);
//
//	glUseProgram(0);
//	glBindTexture(GL_TEXTURE_2D, texture_binding);

	sem_post(&output->vsync_semaphore);
	return true;
}

uint32_t flutter_fbo_callback(void* userdata) {
	auto* output = static_cast<flutland_output*>(userdata);
	uint32_t fb = output->fix_y_flip_state.offscreen_framebuffer;
//	uint32_t fb = wlr_gles2_renderer_get_current_fbo(output->server->renderer);
	return fb;
}

void vsync_callback(void* userdata, intptr_t baton) {
	auto* output = static_cast<flutland_output*>(userdata);
//	std::clog << "VSYNC_CALLBACK" << std::endl;

	pthread_mutex_lock(&output->baton_mutex);
	output->new_baton = true;
	output->baton = baton;
	pthread_mutex_unlock(&output->baton_mutex);
}

bool flutter_gl_external_texture_frame_callback(void* userdata, int64_t texture_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out) {
//	std::clog << "WW_EXTERNAL_TEXTURE_FRAME" << std::endl;

	auto* output = static_cast<flutland_output*>(userdata);
	auto* texture = (struct wlr_texture*) texture_id;
	texture_out->target = GL_TEXTURE_2D;

	struct wlr_gles2_texture_attribs attribs{};
	wlr_gles2_texture_get_attribs(texture, &attribs);
	texture_out->name = attribs.tex;

	texture_out->width = texture->width;
	texture_out->height = texture->height;

	texture_out->format = GL_RGBA8;

//	std::clog << "Texture width: " << texture->width << std::endl;
//	std::clog << "Texture height: " << texture->height << std::endl;

	return true;
}

void start_rendering(void* userdata) {
	auto* output = static_cast<flutland_output*>(userdata);
	struct wlr_renderer* renderer = output->server->renderer;

	if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	wlr_renderer_begin(renderer, width, height);

	bind_offscreen_framebuffer(&output->fix_y_flip_state);
}

void flutter_execute_platform_tasks(void* data) {
	__FlutterEngineFlushPendingTasksNow();
}

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata) {
	auto* output = static_cast<flutland_output*>(userdata);

	if (message->struct_size != sizeof(FlutterPlatformMessage)) {
		std::cerr << "ERROR: Invalid message size received. Expected: "
		          << sizeof(FlutterPlatformMessage) << " but received "
		          << message->struct_size;
		return;
	}

	output->message_dispatcher.HandleMessage(
			*message, [] {}, [] {});

//	std::clog << "Dispatching" << std::endl;
}