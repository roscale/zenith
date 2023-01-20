#include "scoped_wlr_buffer.hpp"

std::shared_ptr<wlr_buffer> scoped_wlr_buffer(wlr_buffer* buffer) {
	return {wlr_buffer_lock(buffer), [](wlr_buffer* buffer) {
		wlr_buffer_unlock(buffer);
	}};
}

std::shared_ptr<wlr_gles2_buffer> scoped_wlr_gles2_buffer(wlr_gles2_buffer* gles2_buffer) {
	wlr_buffer_lock(gles2_buffer->buffer);
	return {gles2_buffer, [](wlr_gles2_buffer* gles2_buffer) {
		wlr_buffer_unlock(gles2_buffer->buffer);
	}};
}
