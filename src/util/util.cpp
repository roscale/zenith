#include "util.hpp"

bool wlr_box_equal(const wlr_box& a, const wlr_box& b) {
	return a.x == b.x and
	       a.y == b.y and
	       a.width == b.width and
	       a.height == b.height;
}
