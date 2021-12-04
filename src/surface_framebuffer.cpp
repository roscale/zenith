#include "surface_framebuffer.hpp"
#include <cassert>
#include <iostream>

SurfaceFramebuffer::SurfaceFramebuffer(size_t width, size_t height)
	  : width(width), height(height) {

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	// Abort if the framebuffer was not correctly created.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE && "Incomplete framebuffer");

	// Restore context state.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindTexture(GL_TEXTURE_2D, texture_binding);
}

void SurfaceFramebuffer::resize(size_t new_width, size_t new_height) {
	width = new_width;
	height = new_height;

	// Backup context state.
	GLint texture_binding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) width, (int) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// Restore context state.
	glBindTexture(GL_TEXTURE_2D, texture_binding);
}

SurfaceFramebuffer::~SurfaceFramebuffer() {
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &framebuffer);
}
