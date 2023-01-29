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
Slot<T>::Slot(std::shared_ptr<T> buffer) : buffer{buffer} {
}

template<class T>
SwapChain<T>::SwapChain(const std::array<std::shared_ptr<T>, 4>& buffers) {
	read_buffer_1 = std::make_shared<Slot<T>>(buffers[0]);
	read_buffer_2 = std::make_shared<Slot<T>>(buffers[1]);
	latest_buffer = std::make_shared<Slot<T>>(buffers[2]);
	write_buffer = std::make_shared<Slot<T>>(buffers[3]);
}

template<class T>
T* SwapChain<T>::start_write() {
	std::scoped_lock lock(mutex);
	return write_buffer->buffer.get();
}

template<class T>
array_view<FlutterRect> SwapChain<T>::get_damage_regions() {
	std::scoped_lock lock(mutex);
	auto& damage = write_buffer->damage_regions;
	return array_view<FlutterRect>(damage.data(), damage.size());
}

template<class T>
void SwapChain<T>::end_write(array_view<FlutterRect> damage) {
	std::scoped_lock lock(mutex);
	write_buffer->damage_regions = std::vector(damage.begin(), damage.end());
	latest_buffer->damage_regions.insert(latest_buffer->damage_regions.end(), damage.begin(), damage.end());
	read_buffer_1->damage_regions.insert(read_buffer_1->damage_regions.end(), damage.begin(), damage.end());
	read_buffer_2->damage_regions.insert(read_buffer_2->damage_regions.end(), damage.begin(), damage.end());

	std::swap(write_buffer, latest_buffer);
	new_buffer_available = true;
}

template<class T>
T* SwapChain<T>::start_read() {
	std::scoped_lock lock(mutex);
	if (new_buffer_available) {
		// read1 read2 latest write -> latest read1 read2 write
		std::swap(latest_buffer, read_buffer_1);
		std::swap(latest_buffer, read_buffer_2);
		new_buffer_available = false;
	}
	return read_buffer_1->buffer.get();
}

template<class T>
SwapChain<T>::~SwapChain() {
	std::scoped_lock lock(mutex);
	read_buffer_1 = nullptr;
	read_buffer_2 = nullptr;
	write_buffer = nullptr;
	latest_buffer = nullptr;
}
