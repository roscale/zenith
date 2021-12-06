#include "flutter_callbacks.hpp"
#include "zenith_structs.hpp"

extern "C" {
#define static
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_output.h>
#undef static
}

#include <GL/gl.h>
#include <cassert>
#include <iostream>
#include <filesystem>

FlutterEngine run_flutter(ZenithOutput* output) {
	FlutterRendererConfig config = {};
	config.type = kOpenGL;
	config.open_gl.struct_size = sizeof(config.open_gl);
	config.open_gl.make_current = flutter_make_current;
	config.open_gl.clear_current = flutter_clear_current;
	config.open_gl.present = flutter_present;
	config.open_gl.fbo_callback = flutter_fbo_callback;
	config.open_gl.gl_external_texture_frame_callback = flutter_gl_external_texture_frame_callback;
	config.open_gl.make_resource_current = flutter_make_resource_current;

#ifdef DEBUG
	FlutterProjectArgs args = {
		  .struct_size = sizeof(FlutterProjectArgs),
		  .assets_path = "data/flutter_assets",
		  .icu_data_path = "data/icudtl.dat",
		  .platform_message_callback = flutter_platform_message_callback,
		  .vsync_callback = vsync_callback,
	};
#else
	auto absolute_path = std::filesystem::canonical("lib/libapp.so");

	FlutterEngineAOTDataSource data_source = {
		  .type = kFlutterEngineAOTDataSourceTypeElfPath,
		  .elf_path = absolute_path.c_str(),
	};

	FlutterEngineAOTData aot_data;
	FlutterEngineCreateAOTData(&data_source, &aot_data);

	FlutterProjectArgs args = {
		  .struct_size = sizeof(FlutterProjectArgs),
		  .assets_path = "data/flutter_assets",
		  .icu_data_path = "data/icudtl.dat",
		  .platform_message_callback = flutter_platform_message_callback,
		  .vsync_callback = vsync_callback,
		  .aot_data = aot_data,
	};
#endif

	FlutterEngine engine = nullptr;
	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, output, &engine);
	assert(result == kSuccess && engine != nullptr);

	return engine;
}

bool flutter_make_current(void* userdata) {
	auto* output = static_cast<ZenithOutput*>(userdata);
	return wlr_egl_make_current(output->flutter_gl_context);
}

bool flutter_clear_current(void* userdata) {
	auto* output = static_cast<ZenithOutput*>(userdata);
	return wlr_egl_unset_current(output->flutter_gl_context);
}

bool flutter_present(void* userdata) {
	auto* output = static_cast<ZenithOutput*>(userdata);
//	wlr_renderer* renderer = output->server->renderer;

//	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(output->server->renderer);

//	static uint8_t buf[4 * 1024 * 768];
//	glReadPixels(0, 0, 1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, buf);

//	static uint8_t buf;
//	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &buf);

	output->flip_mutex.lock();
	render_to_fbo(&output->fix_y_flip, output->present_fbo->framebuffer);
	output->flip_mutex.unlock();

	glFlush();

//	wlr_renderer_end(renderer);
//	wlr_output_commit(output->wlr_output);

//	sem_post(&output->vsync_semaphore);
	return true;
}

uint32_t flutter_fbo_callback(void* userdata) {
	auto* output = static_cast<ZenithOutput*>(userdata);
	return output->fix_y_flip.offscreen_framebuffer;
}

void vsync_callback(void* userdata, intptr_t baton) {
	auto* output = static_cast<ZenithOutput*>(userdata);

	pthread_mutex_lock(&output->baton_mutex);
	assert(output->new_baton == false);
	output->new_baton = true;
	output->baton = baton;
	pthread_mutex_unlock(&output->baton_mutex);
}

bool flutter_gl_external_texture_frame_callback(void* userdata, int64_t view_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out) {
	auto* output = static_cast<ZenithOutput*>(userdata);

	output->server->surface_framebuffers_mutex.lock();
//
	auto surface_framebuffer_it = output->server->surface_framebuffers.find(view_id);
//	if (surface_framebuffer_it == output->surface_framebuffers.end()) {
//		output->surface_framebuffers_mutex.unlock();
//		return false;
//	}

	//	if (surface_framebuffer_it == output->surface_framebuffers.end()) {
//		auto inserted_pair = output->surface_framebuffers.insert(
//			  std::pair<wlr_texture*, std::unique_ptr<SurfaceFramebuffer>>(
//					texture,
//					std::make_unique<SurfaceFramebuffer>(texture->width, texture->height)
//			  )
//		);
//		surface_framebuffer_it = inserted_pair.first;
//	}
//

//	wlr_gles2_texture_attribs attribs{};
//	wlr_gles2_texture_get_attribs(texture, &attribs);
//
//	output->render_to_texture_shader->render(attribs.tex, texture->width, texture->height,
//	                                         surface_framebuffer_it->second->framebuffer);

	texture_out->target = GL_TEXTURE_2D;
	texture_out->name = surface_framebuffer_it->second->texture;

	texture_out->width = surface_framebuffer_it->second->width;
	texture_out->height = surface_framebuffer_it->second->height;

	texture_out->format = GL_RGBA8;

	output->server->surface_framebuffers_mutex.unlock();

	return true;
}

void flutter_execute_platform_tasks(void* data) {
	__FlutterEngineFlushPendingTasksNow();
}

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata) {
	auto* output = static_cast<ZenithOutput*>(userdata);

	if (message->struct_size != sizeof(FlutterPlatformMessage)) {
		std::cerr << "ERROR: Invalid message size received. Expected: "
		          << sizeof(FlutterPlatformMessage) << " but received "
		          << message->struct_size;
		return;
	}

	output->message_dispatcher.HandleMessage(
		  *message, [] {}, [] {});
}

bool flutter_make_resource_current(void* userdata) {
	auto* output = static_cast<ZenithOutput*>(userdata);
	return wlr_egl_make_current(output->flutter_resource_gl_context);
}