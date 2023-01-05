//
// Created by roscale on 1/3/23.
//

#include "channel.hpp"

void channel::write(GLint data) {
	std::unique_lock<std::mutex> lock(buffer_mutex);
	buffer = data;
	data_avail = true;
	cv.notify_all();
}

GLint channel::read() {
	std::unique_lock<std::mutex> lock(buffer_mutex);
	cv.wait(lock, [&]() { return data_avail; });
	GLint item = buffer;
	data_avail = false;
	return item;
}
