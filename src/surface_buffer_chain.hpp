#pragma once

extern "C" {
#include <wlr/types/wlr_buffer.h>
}

struct SurfaceBufferChain {
	wlr_buffer* render_buffer = {};
	wlr_buffer* newest_buffer = {};

	[[nodiscard]] wlr_buffer* start_rendering();

	void finish_rendering();

	void commit_new_buffer(wlr_buffer* buffer);

	virtual ~SurfaceBufferChain();
};
