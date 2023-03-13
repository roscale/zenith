#pragma once

#include <cstdint>
#include <pixman-1/pixman.h>
#include <vector>
#include <optional>

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
}

struct XdgSurfaceCommitMessage {
	wlr_xdg_surface_role role;
	/* Visible bounds */
	int x, y;
	int width, height;
};

struct XdgPopupCommitMessage {
	int64_t parent_id;
	/* Geometry related to the parent */
	int x, y;
	int width, height;
};

enum class SurfaceRole {
	NONE = 0,
	XDG_SURFACE = 1,
	SUBSURFACE = 2,
};

struct SubsurfaceParentState {
	int64_t id;
	int32_t x, y;
};

enum class ToplevelDecoration {
	NONE = 0,
	CLIENT_SIZE = 1,
	SERVER_SIZE = 2,
};

struct SurfaceCommitMessage {
	size_t view_id;
	std::shared_ptr<wlr_buffer> buffer;
	struct {
		SurfaceRole role;
		int texture_id;
		int x, y;
		int width, height;
		int32_t scale;
		pixman_box32_t input_region;
	} surface;
	std::vector<SubsurfaceParentState> subsurfaces_below;
	std::vector<SubsurfaceParentState> subsurfaces_above;
	std::optional<XdgSurfaceCommitMessage> xdg_surface;
	std::optional<XdgPopupCommitMessage> xdg_popup;
	std::optional<ToplevelDecoration> toplevel_decoration;
	std::optional<std::string> toplevel_title;
};

enum class TextInputEventType {
	enable,
	disable,
	commit,
};

struct KeyboardKeyEventMessage {
	wlr_event_keyboard_key event;
	xkb_keycode_t scan_code;
	xkb_keysym_t keysym;
	uint32_t modifiers;
};
