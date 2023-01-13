#pragma once

#include "zenith_xdg_surface.hpp"

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct ZenithXdgPopup {
	ZenithXdgPopup(wlr_xdg_popup* xdg_popup, std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface);

	wlr_xdg_popup* xdg_popup;
	std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface;
};
