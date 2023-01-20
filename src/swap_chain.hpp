#pragma once

#include <memory>

extern "C" {
#include <wlr/types/wlr_buffer.h>
}

template<class T>
struct SwapChain {
	explicit SwapChain(const std::array<std::shared_ptr<T>, 4>& buffers);

	/*
	 * We need 2 read buffers because when start_read is called, the monitor might not have finished
	 * scanning out the previous frame. We don't want the frame to be picked up right away for writing.
	 * That's why, instead of just swapping read1 and latest, we do a circular swap between read1, read2, and latest.
	 * read1 = latest
	 * read2 = read1
	 * latest = read2
	 */
	std::shared_ptr<T> read_buffer_1 = {};
	std::shared_ptr<T> read_buffer_2 = {};
	std::shared_ptr<T> write_buffer = {};
	std::shared_ptr<T> latest_buffer = {};

	bool new_buffer_available = false;

	[[nodiscard]] T* start_write();

	void end_write();

	[[nodiscard]] T* start_read();

	virtual ~SwapChain();
};

#include "swap_chain.tpp"
