#pragma once

#include <memory>

extern "C" {
#include <wlr/types/wlr_buffer.h>
}

template<class T>
struct SurfaceBufferChain {
	std::shared_ptr<T> render_buffer = {};
	std::shared_ptr<T> newest_buffer = {};

	[[nodiscard]] auto start_read() -> T*;

	void end_read();

	void commit_buffer(std::shared_ptr<T> buffer);

	virtual ~SurfaceBufferChain();
};

#include "surface_buffer_chain.tpp"
