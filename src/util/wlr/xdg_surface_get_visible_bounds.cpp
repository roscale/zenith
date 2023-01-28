#include "xdg_surface_get_visible_bounds.hpp"

wlr_box xdg_surface_get_visible_bounds(wlr_xdg_surface* xdg_surface) {
	wlr_box& visible_bounds = xdg_surface->current.geometry;
	if (!wlr_box_empty(&visible_bounds)) {
		return visible_bounds;
	}
	return {
		  .x = 0,
		  .y = 0,
		  .width = xdg_surface->surface->current.buffer_width,
		  .height = xdg_surface->surface->current.buffer_height,
	};
}
