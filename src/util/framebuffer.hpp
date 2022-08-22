#pragma once

#include <epoxy/gl.h>
#include <cstddef>
#include <mutex>
#include <atomic>
#include "offset.hpp"

struct Framebuffer {
	std::mutex mutex{};

	GLuint framebuffer = 0;
	GLuint texture = 0;
	uint32_t width = 0;
	uint32_t height = 0;

	Framebuffer(uint32_t width, uint32_t height);

	void resize(uint32_t new_width, uint32_t new_height);

	~Framebuffer();
};
