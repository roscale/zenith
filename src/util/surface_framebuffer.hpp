#pragma once

#include <cstddef>
#include <mutex>
#include <cassert>
#include <unordered_set>
#include <EGL/egl.h>
#include <epoxy/gl.h>
#include <iostream>

struct Framebuffer {
	GLuint framebuffer = 0;
	GLuint texture = 0;
	size_t width = 0;
	size_t height = 0;

	Framebuffer() = default;

	Framebuffer(GLuint framebuffer, GLuint texture, size_t width, size_t height);
};

struct SurfaceFramebuffer {
	std::mutex mutex{};

	// Triple buffering in order to avoid data races in OpenGL.
	// Let Flutter render from a framebuffer texture while a new frame is put into another framebuffer.
	std::array<Framebuffer, 3> framebuffers{};

	size_t read_index = framebuffers.max_size();
	size_t write_index = framebuffers.max_size();
	size_t newest_index = framebuffers.max_size();

	GLsync read_sync = nullptr;
	GLsync write_sync = nullptr;

	size_t pending_width = 0;
	size_t pending_height = 0;

	SurfaceFramebuffer(size_t width, size_t height);

	void schedule_resize(size_t new_width, size_t new_height);

	bool apply_pending_resize();

	Framebuffer& start_reading() {
		assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
		checkSyncObjects();

//		assert(newest_index != framebuffers.max_size());
		if (newest_index == framebuffers.max_size()) {
			read_index = 0;
			return framebuffers[0];
		}

		read_index = newest_index;

		return framebuffers[newest_index];
	}

	void stop_reading() {
		assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
		checkSyncObjects();
		if (read_sync != nullptr) {
			glDeleteSync(read_sync);
		}
		read_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	Framebuffer& start_writing() {
		assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
		checkSyncObjects();

		size_t unused_fb_index_it = read_index;
		for (size_t i = 0; i < framebuffers.max_size(); i++) {
			if (i != read_index && i != newest_index) {
				unused_fb_index_it = i;
				break;
			}
		}
		write_index = unused_fb_index_it;
		return framebuffers[unused_fb_index_it];
	}

	void stop_writing() {
		assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
		checkSyncObjects();
		if (write_sync != nullptr) {
			glDeleteSync(write_sync);
		}
		write_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	void checkSyncObjects() {
		if (read_sync != nullptr) {
			if (glClientWaitSync(read_sync, 0, 0) == GL_ALREADY_SIGNALED) {
				glDeleteSync(read_sync);
				read_sync = nullptr;
				read_index = framebuffers.max_size();
			}
		}
		if (write_sync != nullptr) {
			if (glClientWaitSync(write_sync, 0, 0) == GL_ALREADY_SIGNALED) {
				glDeleteSync(write_sync);
				write_sync = nullptr;
				newest_index = write_index;
				write_index = framebuffers.max_size();
			}
		}
	}

	~SurfaceFramebuffer();
};
