#pragma once

#include <epoxy/gl.h>
#include <cstddef>
#include <mutex>

struct SurfaceFramebuffer {
	std::mutex mutex{};

	GLuint framebuffer = 0;
	GLuint texture = 0;
	size_t width = 0;
	size_t height = 0;

	size_t pending_width = 0;
	size_t pending_height = 0;

	SurfaceFramebuffer(size_t width, size_t height);

	void schedule_resize(size_t new_width, size_t new_height);

	bool apply_pending_resize();

	~SurfaceFramebuffer();
};