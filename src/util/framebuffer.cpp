#include "framebuffer.hpp"
#include "render_to_texture_shader.hpp"
#include <cassert>
#include <epoxy/gl.h>
#include <epoxy/egl.h>

Framebuffer::Framebuffer(size_t width, size_t height)
	  : width(width), height(height) {

	assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
	// Backup context state.
	GLint framebuffer_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_binding);
	GLint texture_binding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);

	// Create and bind a framebuffer.
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Create a texture and attach it to the framebuffer.
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) width, (int) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	// Abort if the framebuffer was not correctly created.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE && "Incomplete framebuffer");

	// Restore context state.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindTexture(GL_TEXTURE_2D, texture_binding);
}

void Framebuffer::resize(size_t new_width, size_t new_height) {
	assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
	if (width == new_width && height == new_height) {
		return;
	}
	// Backup context state.
	GLint framebuffer_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_binding);
	GLint texture_binding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);

	// Create a texture that having the new dimensions.
	GLuint resized_texture;
	glGenTextures(1, &resized_texture);

	glBindTexture(GL_TEXTURE_2D, resized_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) new_width, (int) new_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Attach the new texture to the framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resized_texture, 0);

	// Copy the current texture into the resized texture to avoid visual artifacts from uninitialized VRAM.
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	RenderToTextureShader::instance()->render(texture, 0, 0, width, height, framebuffer);

	glDeleteTextures(1, &texture);

	width = new_width;
	height = new_height;
	texture = resized_texture;

	// Restore context state.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindTexture(GL_TEXTURE_2D, texture_binding);
}

Framebuffer::~Framebuffer() {
	assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &framebuffer);
}
