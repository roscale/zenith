#include "embedder_callbacks.hpp"
#include "embedder_state.hpp"
#include "server.hpp"

extern "C" {
#include <GLES3/gl3.h>
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
	return attach_framebuffer();
}

GLuint attach_framebuffer() {
	ZenithServer* server = ZenithServer::instance();
	channel<GLuint> fb_chan{};
	server->callable_queue.enqueue([&]() {
		wlr_gles2_buffer* gles2_buffer = server->output->swap_chain.start_write();
		fb_chan.write(gles2_buffer->fbo);
	});
	return fb_chan.read();
}

bool flutter_present(void* userdata) {
	// Wait for the buffer to finish rendering before we commit it to the screen.
	glFinish(); // glFlush still causes flickering on my phone.

	bool success = commit_framebuffer();
	return success;
}

bool commit_framebuffer() {
	ZenithServer* server = ZenithServer::instance();
	channel<bool> success{};

	server->callable_queue.enqueue([&success]() {
		ZenithServer* server = ZenithServer::instance();
		server->output->swap_chain.end_write();
		success.write(true);
	});
	return success.read();
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
	ZenithServer* server = ZenithServer::instance();
	const int64_t& view_id = texture_id;
	channel<wlr_gles2_texture_attribs> texture_attribs{};

	server->callable_queue.enqueue([&]() {
		auto it = server->surface_buffer_chains.find(view_id);
		if (it == server->surface_buffer_chains.end()) {
			texture_attribs.write({});
			return;
		}
		const auto& client_chain = it->second;

		state->buffer_chains_in_use[view_id] = client_chain;

		wlr_buffer* buffer = client_chain->start_rendering();
		if (buffer == nullptr) {
			texture_attribs.write({});
			return;
		}

		wlr_texture* texture = wlr_client_buffer_get(buffer)->texture;
		if (texture == nullptr) {
			texture_attribs.write({});
			return;
		}

		wlr_gles2_texture_attribs attribs{};
		wlr_gles2_texture_get_attribs(texture, &attribs);
		texture_attribs.write(attribs);
		return;
	});

	wlr_gles2_texture_attribs attribs = texture_attribs.read();
	if (attribs.tex == 0) {
		return false;
	}

//	std::cout << "target " << attribs.target << std::endl;

	texture_out->target = attribs.target;
	texture_out->format = GL_RGBA8;
	texture_out->name = attribs.tex;
	texture_out->user_data = (void*) view_id;
	texture_out->destruction_callback = [](void* user_data) {
		auto* server = ZenithServer::instance();
		auto view_id = reinterpret_cast<int64_t>(user_data);
		server->callable_queue.enqueue([=]() {
			auto& buffer_chains_in_use = server->embedder_state->buffer_chains_in_use;

			auto it = buffer_chains_in_use.find(view_id);
			if (it != buffer_chains_in_use.end()) {
				it->second->finish_rendering();
			}
		});
	};

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
