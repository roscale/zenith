#include "flutter_callbacks.hpp"
#include "flutter_engine_state.hpp"
#include "server.hpp"

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

bool flutter_make_current(void* userdata) {
	auto* state = static_cast<FlutterEngineState*>(userdata);
	return wlr_egl_make_current(state->flutter_gl_context);
}

bool flutter_clear_current(void* userdata) {
	auto* state = static_cast<FlutterEngineState*>(userdata);
	return wlr_egl_unset_current(state->flutter_gl_context);
}

bool flutter_present(void* userdata) {
	auto* state = static_cast<FlutterEngineState*>(userdata);

	state->flip_mutex.lock();
	render_to_fbo(&state->fix_y_flip, state->present_fbo->framebuffer);
	state->flip_mutex.unlock();

	glFlush(); // Don't remove this line!

	return true;
}

uint32_t flutter_fbo_callback(void* userdata) {
	auto* state = static_cast<FlutterEngineState*>(userdata);
	return state->fix_y_flip.offscreen_framebuffer;
}

void flutter_vsync_callback(void* userdata, intptr_t baton) {
	auto* state = static_cast<FlutterEngineState*>(userdata);

	pthread_mutex_lock(&state->baton_mutex);
	assert(state->new_baton == false);
	state->new_baton = true;
	state->baton = baton;
	pthread_mutex_unlock(&state->baton_mutex);
}

bool flutter_gl_external_texture_frame_callback(void* userdata, int64_t view_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out) {
	auto* state = static_cast<FlutterEngineState*>(userdata);
	ZenithServer* server = state->output->server;

	server->surface_framebuffers_mutex.lock();

	auto it = server->surface_framebuffers.find(view_id);
	auto& surface_framebuffer = it->second;

	surface_framebuffer->apply_pending_resize();

	texture_out->target = GL_TEXTURE_2D;
	texture_out->format = GL_RGBA8;
	texture_out->name = surface_framebuffer->texture;

	server->surface_framebuffers_mutex.unlock();

	return true;
}

void flutter_execute_platform_tasks(void* data) {
	__FlutterEngineFlushPendingTasksNow();
}

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata) {
	auto* state = static_cast<FlutterEngineState*>(userdata);

	if (message->struct_size != sizeof(FlutterPlatformMessage)) {
		std::cerr << "ERROR: Invalid message size received. Expected: "
		          << sizeof(FlutterPlatformMessage) << " but received "
		          << message->struct_size;
		return;
	}

	state->message_dispatcher.HandleMessage(
		  *message, [] {}, [] {});
}

bool flutter_make_resource_current(void* userdata) {
	auto* state = static_cast<FlutterEngineState*>(userdata);
	return wlr_egl_make_current(state->flutter_resource_gl_context);
}