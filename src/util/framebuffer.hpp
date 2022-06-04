#pragma once

#include <epoxy/gl.h>
#include <cstddef>
#include <mutex>
#include <atomic>

struct Framebuffer {
	std::mutex mutex{};

	GLuint framebuffer = 0;
	GLuint texture = 0;
	size_t width = 0;
	size_t height = 0;

	Framebuffer(size_t width, size_t height);

	void resize(size_t new_width, size_t new_height);

	~Framebuffer();
};