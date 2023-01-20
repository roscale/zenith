#pragma once

#include <memory>

extern "C" {
#include <wlr/types/wlr_buffer.h>
}

template<class T>
struct DoubleBuffering {
	std::shared_ptr<T> render_buffer = {};
	std::shared_ptr<T> newest_buffer = {};

	[[nodiscard]] T* start_rendering();

	void finish_rendering();

	void commit_new_buffer(std::shared_ptr<T> buffer);

	virtual ~DoubleBuffering();
};

#include "double_buffering.tpp"
