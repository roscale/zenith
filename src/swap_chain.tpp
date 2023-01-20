#pragma once

#include <memory>
#include <thread>
#include "time.hpp"

extern "C" {
#include <wlr/render/allocator.h>
#define static
#include <wlr/render/gles2.h>
#undef static
}

template<class T>
SwapChain<T>::SwapChain(const std::array<std::shared_ptr<T>, 4>& buffers) {
	read_buffer_1 = buffers[0];
	read_buffer_2 = buffers[1];
	latest_buffer = buffers[2];
	write_buffer = buffers[3];
}

template<class T>
T* SwapChain<T>::start_write() {
	return write_buffer.get();
}

template<class T>
void SwapChain<T>::end_write() {
	std::swap(write_buffer, latest_buffer);
	new_buffer_available = true;
}

template<class T>
T* SwapChain<T>::start_read() {
	if (new_buffer_available) {
		// read1 read2 latest write -> latest read1 read2 write
		std::swap(latest_buffer, read_buffer_1);
		std::swap(latest_buffer, read_buffer_2);
		new_buffer_available = false;
	}
	return read_buffer_1.get();
}

template<class T>
SwapChain<T>::~SwapChain() {
	read_buffer_1 = nullptr;
	read_buffer_2 = nullptr;
	write_buffer = nullptr;
	latest_buffer = nullptr;
}
