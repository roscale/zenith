#pragma once

#include <xkbcommon/xkbcommon.h>
#include <cassert>
#include <string>
#include <optional>

extern "C" {
#include <wlr/types/wlr_keyboard.h>
}

struct KeyCombination {
	xkb_mod_mask_t modifiers;
	xkb_keycode_t keycode;
};

// Given a character as a string, it returns the modifiers and keycode needed to generate that character.
// Sometimes there are multiple key combinations that lead to the same character, but the first one is returned.
std::optional<KeyCombination> string_to_keycode(const char* string, xkb_state* state);

uint32_t wlr_modifiers_to_gtk(uint32_t mods);
