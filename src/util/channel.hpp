#pragma once

#include <epoxy/gl.h>
#include <mutex>
#include <condition_variable>

class channel {
public:
	GLint buffer{};
	std::mutex buffer_mutex{}; // controls access to buffer
	std::condition_variable cv{};
	bool data_avail = false;

	void write(GLint data);

	GLint read();
};