#include "string_to_keycode.hpp"
#include <tuple>

std::optional<KeyCombination> string_to_keycode(const char* string, xkb_state* state) {
	std::optional<KeyCombination> return_value{};
	xkb_keymap* keymap = xkb_state_get_keymap(state);

	auto tuple = std::make_tuple(string, state, &return_value);
	xkb_keymap_key_for_each(
		  keymap,
		  [](struct xkb_keymap* keymap, xkb_keycode_t key, void* data) {
			  auto* tuple_ptr = static_cast<decltype(tuple)*>(data);
			  auto [string, state, return_value] = *tuple_ptr;

			  xkb_layout_index_t layout = xkb_state_key_get_layout(state, key);
			  xkb_level_index_t levels = xkb_keymap_num_levels_for_key(keymap, key, layout);
			  for (xkb_level_index_t level = 0; level < levels; level++) {
				  const xkb_keysym_t* symbols;
				  int symbol_count = xkb_keymap_key_get_syms_by_level(keymap, key, layout, level, &symbols);

				  for (int i = 0; i < symbol_count; i++) {
					  xkb_keysym_t symbol = symbols[i];

					  char buffer[32];
					  int n = xkb_keysym_to_utf8(symbol, buffer, sizeof(buffer));
					  if (n <= 0) {
						  continue;
					  }

					  std::string sym_str = buffer;
					  if (sym_str == string) {
						  xkb_mod_mask_t mask;
						  size_t n = xkb_keymap_key_get_mods_for_level(keymap, key, layout, level, &mask, 1);
						  if (n == 0) {
							  continue;
						  }

						  *return_value = KeyCombination{
								.modifiers = mask,
								.keycode = key,
						  };
						  return;
					  }
				  }
			  }
		  },
		  &tuple
	);
	return return_value;
}
