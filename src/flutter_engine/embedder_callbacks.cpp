#include "embedder_callbacks.hpp"
#include "embedder_state.hpp"
#include "server.hpp"

extern "C" {
#define static
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#undef static
}

#include <cassert>
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>

bool flutter_make_current(void* userdata) {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_make_current(state->flutter_gl_context);
}

bool flutter_clear_current(void* userdata) {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_unset_current(state->flutter_gl_context);
}

uint32_t flutter_fbo_with_frame_info_callback(void* userdata, const FlutterFrameInfo* frame_info) {
	auto* state = static_cast<EmbedderState*>(userdata);
	const auto& output = state->server->output;

	eventfd_write(output->attach_event_fd, 1);
	GLint fb;
	read(output->attach_event_return_pipes[0], &fb, sizeof fb);

	return fb;
}

bool flutter_present(void* userdata) {
	// https://community.arm.com/cfs-file/__key/telligent-evolution-components-attachments/01-2066-00-00-00-00-42-84/Whitepaper_2D00_ThreadSync.pdf
	auto* state = static_cast<EmbedderState*>(userdata);
	const auto& output = state->server->output;

	GLint fb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
	glFinish(); // glFlush still causes flickering on my phone.

	eventfd_write(output->commit_event_fd, 1);
	bool success;
	read(output->commit_event_return_pipes[0], &success, sizeof success);

	return success;
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
	texture_out->target = GL_TEXTURE_2D;
	// From epoxy/gl.h
	// If I include this header I have redefinition errors.
	const uint32_t GL_RGBA8 = 0x8058;
	texture_out->format = GL_RGBA8;
	texture_out->name = texture_id;

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

/*
 * The default rendering is done upside down for some reason.
 * This flips the rendering on the x-axis.
 */
FlutterTransformation flutter_surface_transformation(void* data) {
	auto* state = static_cast<EmbedderState*>(data);
	double height = state->server->output->wlr_output->height;

	return FlutterTransformation{
		  1.0, 0.0, 0.0, 0.0, -1.0, height, 0.0, 0.0, 1.0,
	};
}
