#pragma once

#include <mutex>
#include <condition_variable>

/*
 * A channel for sending a single value.
 */
template<typename T>
class channel {
	std::mutex mutex{};
	std::condition_variable cv{};
	T data;
	bool data_available = false;

public:
	void write(T value) {
		std::unique_lock<std::mutex> lock(mutex);
		data = value;
		data_available = true;
		cv.notify_all();
	}

	T read() {
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock, [&]() { return data_available; });
		data_available = false;
		return data;
	}
};
