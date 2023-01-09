#pragma once

#include <wlr/util/box.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <optional>
#include "encodable_value.h"
#include "standard_method_codec.h"
#include "binary_messenger.hpp"

using namespace flutter;

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

enum SurfaceRole {
	NONE = 0,
	XDG_SURFACE = 1,
	SUBSURFACE = 2,
};

struct SurfaceCommitMessage {
	size_t view_id;
	struct {
		SurfaceRole role;
		int texture_id;
		int x, y;
		int width, height;
		int32_t scale;
	} surface;
	std::optional<XdgSurfaceCommitMessage> xdg_surface;
	std::optional<XdgPopupCommitMessage> xdg_popup;
};

void send_window_mapped(BinaryMessenger& messenger,
                        size_t view_id, size_t texture_id, int surface_width, int surface_height,
                        wlr_box& visible_bounds);

void send_popup_mapped(BinaryMessenger& messenger,
                       size_t view_id, size_t texture_id, size_t parent_view_id, wlr_box& surface_box,
                       wlr_box& visible_bounds);

void send_window_unmapped(BinaryMessenger& messenger, size_t view_id);

void send_popup_unmapped(BinaryMessenger& messenger, size_t view_id);

void send_configure_xdg_surface(BinaryMessenger& messenger,
                                size_t view_id, enum wlr_xdg_surface_role surface_role,
                                std::optional<std::reference_wrapper<wlr_box>> new_visible_bounds,
                                std::optional<int> new_texture_id,
                                std::optional<std::pair<int, int>> new_surface_size,
                                std::optional<std::pair<int, int>> new_popup_position);

void send_text_input_enabled(BinaryMessenger& messenger, size_t view_id);

void send_text_input_disabled(BinaryMessenger& messenger, size_t view_id);

void send_text_input_committed(BinaryMessenger& messenger, size_t view_id);

void change_view_texture(BinaryMessenger& messenger, size_t view_id, size_t texture_id);

void send_surface_commit(BinaryMessenger& messenger, const SurfaceCommitMessage& message);

void send_xdg_surface_map(BinaryMessenger& messenger, size_t view_id);

void send_xdg_surface_unmap(BinaryMessenger& messenger, size_t view_id);
