#pragma once

#include <memory>
#include "util/wlr/wlr_helpers.hpp"

extern "C" {
#include <wlr/types/wlr_buffer.h>
}

using Deleter = std::function<void(wlr_buffer* buffer)>;
static Deleter empty_deleter = [](wlr_buffer* buffer) {};

// Wraps a wlr_buffer into a shared_ptr which unlocks the buffer when no instances of it remain.
std::shared_ptr<wlr_buffer> scoped_wlr_buffer(wlr_buffer* buffer, const Deleter& deleter = empty_deleter);

std::shared_ptr<wlr_gles2_buffer> scoped_wlr_gles2_buffer(wlr_gles2_buffer* gles_2_buffer);
