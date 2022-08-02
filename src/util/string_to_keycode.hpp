#pragma once

#include <xkbcommon/xkbcommon.h>
#include <cassert>
#include <string>
#include <optional>

struct KeyCombination {
	xkb_mod_mask_t modifiers;
	xkb_keycode_t keycode;
};

// Given a character as a string, it returns the modifiers and keycode needed to generate that character.
// Sometimes there are multiple key combinations that lead to the same character, but the first one is returned.
std::optional<KeyCombination> string_to_keycode(const char* string, xkb_state* state);