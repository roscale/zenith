#include <epoxy/gl.h>
#include "surface_framebuffer.hpp"
#include "render_to_texture_shader.hpp"
#include <cassert>
#include <iostream>
#include <thread>

SurfaceFramebuffer::SurfaceFramebuffer(size_t width, size_t height)
	  : pending_width(width), pending_height(height) {

	// Backup context state.
	GLint framebuffer_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_binding);
	GLint texture_binding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);

	for (size_t i = 0; i < framebuffers.max_size(); i++) {
		GLuint framebuffer;
		GLuint texture;

		// Create and bind a framebuffer.
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// Create a texture and attach it to the framebuffer.
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) width, (int) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		// Abort if the framebuffer was not correctly created.
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		assert(status == GL_FRAMEBUFFER_COMPLETE && "Incomplete framebuffer");

		Framebuffer fb(framebuffer, texture, width, height);
		framebuffers[i] = fb;
	}

	// Restore context state.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindTexture(GL_TEXTURE_2D, texture_binding);
}

void SurfaceFramebuffer::schedule_resize(size_t new_width, size_t new_height) {
	pending_width = new_width;
	pending_height = new_height;
}

bool SurfaceFramebuffer::apply_pending_resize() {
	Framebuffer& fb = framebuffers[write_index];

	if (fb.width == pending_width && fb.height == pending_height) {
		return false;
	}

	// Backup context state.
	GLint framebuffer_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_binding);
	GLint texture_binding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);

	// Create a texture and attach it to the framebuffer.
	GLuint resized_texture;
	glGenTextures(1, &resized_texture);

	glBindTexture(GL_TEXTURE_2D, resized_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) pending_width, (int) pending_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
	             nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, fb.framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resized_texture, 0);

	// Copy the current texture into the resized texture to avoid flickering.
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	RenderToTextureShader::instance()->render(fb.texture, 0, 0, fb.width, fb.height, fb.framebuffer);

	glDeleteTextures(1, &fb.texture);

	fb.width = pending_width;
	fb.height = pending_height;
	fb.texture = resized_texture;

	// Restore context state.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindTexture(GL_TEXTURE_2D, texture_binding);

	return true;
}

SurfaceFramebuffer::~SurfaceFramebuffer() {
	assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
	for (size_t i = 0; i < framebuffers.max_size(); i++) {
		Framebuffer& fb = framebuffers[i];
		glDeleteTextures(1, &fb.texture);
		glDeleteFramebuffers(1, &fb.framebuffer);
	}
}

Framebuffer::Framebuffer(GLuint framebuffer, GLuint texture, size_t width, size_t height) : framebuffer(framebuffer),
                                                                                            texture(texture),
                                                                                            width(width),
                                                                                            height(height) {
}
