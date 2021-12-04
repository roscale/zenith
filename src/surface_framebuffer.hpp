#pragma once

#include <GLES2/gl2.h>
#include <cstddef>

struct SurfaceFramebuffer {
	GLuint framebuffer = 0;
	GLuint texture = 0;
	size_t width = 0;
	size_t height = 0;

	SurfaceFramebuffer(size_t width, size_t height);

	void resize(size_t width, size_t height);

	~SurfaceFramebuffer();
};