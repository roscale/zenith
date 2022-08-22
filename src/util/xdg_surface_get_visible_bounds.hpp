#pragma once

extern "C" {
#include <wlr/util/box.h>
#include <wlr/types/wlr_xdg_shell.h>
}

// Gets the visible bounds of an xdg_surface.
// Doesn't take subsurfaces into account, which means that subsurfaces extending outside the
// bounds of the xdg_surface are ignored.
wlr_box xdg_surface_get_visible_bounds(wlr_xdg_surface* xdg_surface);