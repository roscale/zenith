#pragma once

#include <thread>

// Avoid circular dependency on `server.hpp`.
bool is_main_thread();

template<class T>
T* DoubleBuffering<T>::start_rendering() {
	assert(is_main_thread());
	render_buffer = newest_buffer;
	return render_buffer.get();
}

template<class T>
void DoubleBuffering<T>::finish_rendering() {
	assert(is_main_thread());
	render_buffer = nullptr;
}

template<class T>
void DoubleBuffering<T>::commit_new_buffer(std::shared_ptr<T> buffer) {
	assert(is_main_thread());
	newest_buffer = std::move(buffer);
}

template<class T>
DoubleBuffering<T>::~DoubleBuffering() {
	assert(is_main_thread());
	render_buffer = nullptr;
	newest_buffer = nullptr;
}
