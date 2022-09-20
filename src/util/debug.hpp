#pragma once

extern "C" {
#include <wlr/types/wlr_surface.h>
}

void print_surface_tree_debug_info(wlr_surface* surface, int x = 0, int y = 0,
                                   int indents = 0, wlr_subsurface* subsurface = nullptr);

[[maybe_unused]] void dump_framebuffer_to_file(const char* filename, int framebuffer, int width, int height);
