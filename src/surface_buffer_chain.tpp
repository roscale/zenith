#pragma once

#include <thread>

// Avoid circular dependency on `server.hpp`.
bool is_main_thread();

template<class T>
T* SurfaceBufferChain<T>::start_read() {
	assert(is_main_thread());
	render_buffer = newest_buffer;
	return render_buffer.get();
}

template<class T>
void SurfaceBufferChain<T>::end_read() {
	assert(is_main_thread());
	render_buffer = nullptr;
}

template<class T>
void SurfaceBufferChain<T>::commit_buffer(std::shared_ptr<T> buffer) {
	assert(is_main_thread());
	newest_buffer = std::move(buffer);
}

template<class T>
SurfaceBufferChain<T>::~SurfaceBufferChain() {
	assert(is_main_thread());
	render_buffer = nullptr;
	newest_buffer = nullptr;
}
