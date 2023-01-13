#include "zenith_xdg_popup.hpp"

ZenithXdgPopup::ZenithXdgPopup(wlr_xdg_popup* xdg_popup, std::shared_ptr<ZenithXdgSurface> zenith_xdg_surface)
	  : xdg_popup{xdg_popup}, zenith_xdg_surface(std::move(zenith_xdg_surface)) {
}
