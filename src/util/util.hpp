#pragma once

extern "C" {
#include <wlr/types/wlr_box.h>
}

bool wlr_box_equal(const wlr_box& a, const wlr_box& b);