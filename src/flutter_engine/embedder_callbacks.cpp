#include "embedder_callbacks.hpp"
#include "embedder_state.hpp"
#include "server.hpp"
#include "gl_mutex.hpp"

extern "C" {
#define static
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#undef static
}

#include <epoxy/gl.h>
#include <cassert>
#include <iostream>

bool flutter_make_current(void* userdata) {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_make_current(state->flutter_gl_context);
}

bool flutter_clear_current(void* userdata) {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_unset_current(state->flutter_gl_context);
}

bool flutter_present(void* userdata) {
	// https://community.arm.com/cfs-file/__key/telligent-evolution-components-attachments/01-2066-00-00-00-00-42-84/Whitepaper_2D00_ThreadSync.pdf

	auto* state = static_cast<EmbedderState*>(userdata);

	std::scoped_lock lock(state->copy_framebuffer->mutex);
	GLScopedLock gl_lock(state->output_gl_mutex);

	Framebuffer& output_framebuffer = *state->output_framebuffer;
	Framebuffer& copy_framebuffer = *state->copy_framebuffer;

	copy_framebuffer.resize(output_framebuffer.width, output_framebuffer.height);
	RenderToTextureShader::instance()->render(output_framebuffer.texture, 0, 0, output_framebuffer.width,
	                                          output_framebuffer.height, copy_framebuffer.framebuffer);

	return true;
}

uint32_t with_frame_info_callback(void* userdata, const FlutterFrameInfo* frame_info) {
	auto* state = static_cast<EmbedderState*>(userdata);

	state->output_framebuffer->resize(frame_info->size.width, frame_info->size.height);

	// No need for synchronization because the framebuffer field never changes after the object construction.
	return state->output_framebuffer->framebuffer;
}

void flutter_vsync_callback(void* userdata, intptr_t baton) {
	auto* state = static_cast<EmbedderState*>(userdata);

	std::scoped_lock lock(state->baton_mutex);

	assert(state->new_baton == false);
	state->new_baton = true;
	state->baton = baton;
}

bool flutter_gl_external_texture_frame_callback(void* userdata, int64_t texture_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out) {
	auto* state = static_cast<EmbedderState*>(userdata);
	ZenithServer* server = state->server;

	std::shared_ptr<Framebuffer> surface_framebuffer;
	{
		std::scoped_lock lock(server->surface_framebuffers_mutex);

		auto it = server->surface_framebuffers.find(texture_id);
		if (it == server->surface_framebuffers.end()) {
			// This function could be called any time so we better check if the framebuffer still exists.
			// Asynchronicity can be a pain sometimes.
			return false;
		}
		surface_framebuffer = it->second;
	}

	std::scoped_lock lock(surface_framebuffer->mutex);

	texture_out->target = GL_TEXTURE_2D;
	texture_out->format = GL_RGBA8;
	texture_out->name = surface_framebuffer->texture;

	return true;
}

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata) {
	auto* state = static_cast<EmbedderState*>(userdata);

	if (message->struct_size != sizeof(FlutterPlatformMessage)) {
		std::cerr << "ERROR: Invalid message size received. Expected: "
		          << sizeof(FlutterPlatformMessage) << " but received "
		          << message->struct_size;
		return;
	}

	state->message_dispatcher.HandleMessage(*message, [] {}, [] {});
}

bool flutter_make_resource_current(void* userdata) {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_make_current(state->flutter_resource_gl_context);
}

int flutter_execute_expired_tasks_timer(void* data) {
	auto* state = static_cast<EmbedderState*>(data);

	state->platform_task_runner.execute_expired_tasks();
	// I would have preferred to have the delay represented in nanoseconds because I could reschedule
	// an update at exactly the right time for the earliest task to be executed, but we'll just reschedule
	// as fast as possible, every millisecond. This shouldn't be heavy for a CPU anyway.
	wl_event_source_timer_update(state->platform_task_runner_timer, 1);
	return 0;
}