#include <memory>
#include "wlr_helpers.hpp"

extern "C" {
#include <wlr/types/wlr_buffer.h>
}

// Wraps a wlr_buffer into a shared_ptr which unlocks the buffer when no instances of it remain.
std::shared_ptr<wlr_buffer> scoped_wlr_buffer(wlr_buffer* buffer);

std::shared_ptr<wlr_gles2_buffer> scoped_wlr_gles2_buffer(wlr_gles2_buffer* gles_2_buffer);
