#include "surface_buffer_chain.hpp"

wlr_buffer* SurfaceBufferChain::start_rendering() {
	render_buffer = newest_buffer;
	return render_buffer;
}

void SurfaceBufferChain::finish_rendering() {
	if (render_buffer != newest_buffer) {
		// A new buffer is already available, we can release the one that has been rendered.
		if (render_buffer != nullptr) {
			wlr_buffer_unlock(render_buffer);
		}
	}
	render_buffer = nullptr;
}

void SurfaceBufferChain::commit_new_buffer(wlr_buffer* buffer) {
	if (buffer == newest_buffer) {
		return;
	}
	wlr_buffer_lock(buffer);

	if (render_buffer != newest_buffer) {
		// Don't release the buffer that's currently rendering.
		if (newest_buffer != nullptr) {
			wlr_buffer_unlock(newest_buffer);
		}
	}
	newest_buffer = buffer;
}

SurfaceBufferChain::~SurfaceBufferChain() {
	if (render_buffer != nullptr) {
		wlr_buffer_unlock(render_buffer);
	}
	if (newest_buffer != nullptr && newest_buffer != render_buffer) {
		wlr_buffer_unlock(newest_buffer);
	}
}
