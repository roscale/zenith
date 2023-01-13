#include "messages.hpp"

void
send_window_mapped(BinaryMessenger& messenger, size_t view_id, size_t texture_id, int surface_width, int surface_height,
                   wlr_box& visible_bounds) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"),        EncodableValue((int64_t) view_id)},
		  {EncodableValue("texture_id"),     EncodableValue((int64_t) texture_id)},
		  {EncodableValue("surface_width"),  EncodableValue(surface_width)},
		  {EncodableValue("surface_height"), EncodableValue(surface_height)},
		  {EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
				{EncodableValue("x"),      EncodableValue(visible_bounds.x)},
				{EncodableValue("y"),      EncodableValue(visible_bounds.y)},
				{EncodableValue("width"),  EncodableValue(visible_bounds.width)},
				{EncodableValue("height"), EncodableValue(visible_bounds.height)},
		  })},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("window_mapped", message->data(), message->size());
}

void send_popup_mapped(BinaryMessenger& messenger, size_t view_id, size_t texture_id, size_t parent_view_id,
                       wlr_box& surface_box,
                       wlr_box& visible_bounds) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"),        EncodableValue((int64_t) view_id)},
		  {EncodableValue("texture_id"),     EncodableValue((int64_t) texture_id)},
		  {EncodableValue("parent_view_id"), EncodableValue((int64_t) parent_view_id)},
		  {EncodableValue("x"),              EncodableValue(surface_box.x)},
		  {EncodableValue("y"),              EncodableValue(surface_box.y)},
		  {EncodableValue("surface_width"),  EncodableValue(surface_box.width)},
		  {EncodableValue("surface_height"), EncodableValue(surface_box.height)},
		  {EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
				{EncodableValue("x"),      EncodableValue(visible_bounds.x)},
				{EncodableValue("y"),      EncodableValue(visible_bounds.y)},
				{EncodableValue("width"),  EncodableValue(visible_bounds.width)},
				{EncodableValue("height"), EncodableValue(visible_bounds.height)},
		  })},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("popup_mapped", message->data(), message->size());
}

void send_window_unmapped(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("window_unmapped", message->data(), message->size());
}

void send_popup_unmapped(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("popup_unmapped", message->data(), message->size());
}

void send_configure_xdg_surface(BinaryMessenger& messenger, size_t view_id, enum wlr_xdg_surface_role surface_role,
                                std::optional<std::reference_wrapper<wlr_box>> new_visible_bounds,
                                std::optional<int> new_texture_id,
                                std::optional<std::pair<int, int>> new_surface_size,
                                std::optional<std::pair<int, int>> new_popup_position) {
	auto map = EncodableMap{
		  {EncodableValue("view_id"),      EncodableValue((int64_t) view_id)},
		  {EncodableValue("surface_role"), EncodableValue(surface_role)},
	};

	map.insert({EncodableValue("visible_bounds_changed"), EncodableValue(new_visible_bounds.has_value())});
	if (new_visible_bounds) {
		wlr_box& bounds = new_visible_bounds->get();
		map.insert({EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
			  {EncodableValue("x"),      EncodableValue(bounds.x)},
			  {EncodableValue("y"),      EncodableValue(bounds.y)},
			  {EncodableValue("width"),  EncodableValue(bounds.width)},
			  {EncodableValue("height"), EncodableValue(bounds.height)},
		})});
	}

	map.insert({EncodableValue("surface_size_changed"), EncodableValue(new_surface_size.has_value())});
	if (new_surface_size && new_texture_id) {
		map.insert({EncodableValue("texture_id"), EncodableValue(new_texture_id.value())});
		map.insert({EncodableValue("surface_width"), EncodableValue(new_surface_size->first)});
		map.insert({EncodableValue("surface_height"), EncodableValue(new_surface_size->second)});
	}

	map.insert({EncodableValue("popup_position_changed"), EncodableValue(new_popup_position.has_value())});
	if (new_popup_position) {
		map.insert({EncodableValue("x"), EncodableValue(new_popup_position->first)});
		map.insert({EncodableValue("y"), EncodableValue(new_popup_position->second)});
	}

	auto value = EncodableValue(map);
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("configure_surface", message->data(), message->size());
}

void send_text_input_enabled(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		  {EncodableValue("type"),    EncodableValue("enable")},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("text_input_events", message->data(), message->size());
}

void send_text_input_disabled(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		  {EncodableValue("type"),    EncodableValue("disable")},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("text_input_events", message->data(), message->size());
}

void send_text_input_committed(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		  {EncodableValue("type"),    EncodableValue("commit")},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("text_input_events", message->data(), message->size());
}

void change_view_texture(BinaryMessenger& messenger, size_t view_id, size_t texture_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"),    EncodableValue((int64_t) view_id)},
		  {EncodableValue("texture_id"), EncodableValue((int64_t) texture_id)},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("change_view_texture", message->data(), message->size());
}

void send_surface_commit(BinaryMessenger& messenger, const SurfaceCommitMessage& message) {
	EncodableList subsurfaces_below{};
	for (const SubsurfaceParentState& state: message.subsurfaces_below) {
		subsurfaces_below.emplace_back(EncodableMap{
			  {EncodableValue("id"), EncodableValue(state.id)},
			  {EncodableValue("x"),  EncodableValue(state.x)},
			  {EncodableValue("y"),  EncodableValue(state.y)},
		});
	}
	EncodableList subsurfaces_above{};
	for (const SubsurfaceParentState& state: message.subsurfaces_above) {
		subsurfaces_above.emplace_back(EncodableMap{
			  {EncodableValue("id"), EncodableValue(state.id)},
			  {EncodableValue("x"),  EncodableValue(state.x)},
			  {EncodableValue("y"),  EncodableValue(state.y)},
		});
	}

	auto map = EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) message.view_id)},
		  {EncodableValue("surface"), EncodableValue(EncodableMap{
				{EncodableValue("role"),              EncodableValue((int64_t) message.surface.role)},
				{EncodableValue("textureId"),         EncodableValue(message.surface.texture_id)},
				{EncodableValue("x"),                 EncodableValue(message.surface.x)},
				{EncodableValue("y"),                 EncodableValue(message.surface.y)},
				{EncodableValue("width"),             EncodableValue(message.surface.width)},
				{EncodableValue("height"),            EncodableValue(message.surface.height)},
				{EncodableValue("scale"),             EncodableValue(message.surface.scale)},
				{EncodableValue("subsurfaces_below"), EncodableValue(std::move(subsurfaces_below))},
				{EncodableValue("subsurfaces_above"), EncodableValue(std::move(subsurfaces_above))},
				{EncodableValue("input_region"),      EncodableValue(EncodableMap{
					  {EncodableValue("x1"), EncodableValue((int64_t) message.surface.input_region.x1)},
					  {EncodableValue("y1"), EncodableValue((int64_t) message.surface.input_region.y1)},
					  {EncodableValue("x2"), EncodableValue((int64_t) message.surface.input_region.x2)},
					  {EncodableValue("y2"), EncodableValue((int64_t) message.surface.input_region.y2)},
				})},
		  })},
	};
	if (message.xdg_surface.has_value()) {
		map.insert({EncodableValue("has_xdg_surface"), EncodableValue(true)});
		map.insert({EncodableValue("xdg_surface"), EncodableValue(EncodableMap{
			  {EncodableValue("role"),   EncodableValue((int64_t) message.xdg_surface->role)},
			  {EncodableValue("x"),      EncodableValue(message.xdg_surface->x)},
			  {EncodableValue("y"),      EncodableValue(message.xdg_surface->y)},
			  {EncodableValue("width"),  EncodableValue(message.xdg_surface->width)},
			  {EncodableValue("height"), EncodableValue(message.xdg_surface->height)},
		})});
	} else {
		map.insert({EncodableValue("has_xdg_surface"), EncodableValue(false)});
	}

	if (message.xdg_popup.has_value()) {
		map.insert({EncodableValue("has_xdg_popup"), EncodableValue(true)});
		map.insert({EncodableValue("xdg_popup"), EncodableValue(EncodableMap{
			  {EncodableValue("parent_id"), EncodableValue(message.xdg_popup->parent_id)},
			  {EncodableValue("x"),         EncodableValue(message.xdg_popup->x)},
			  {EncodableValue("y"),         EncodableValue(message.xdg_popup->y)},
			  {EncodableValue("width"),     EncodableValue(message.xdg_popup->width)},
			  {EncodableValue("height"),    EncodableValue(message.xdg_popup->height)},
		})});
	} else {
		map.insert({EncodableValue("has_xdg_popup"), EncodableValue(false)});
	}

	auto value = EncodableValue(map);
	auto envelope = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("zenith_surface_commit", envelope->data(), envelope->size());
}

void send_xdg_surface_map(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("xdg_surface_map", message->data(), message->size());
}

void send_xdg_surface_unmap(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("xdg_surface_unmap", message->data(), message->size());
}

void send_subsurface_map(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("zenith_subsurface_map", message->data(), message->size());
}

void send_subsurface_unmap(BinaryMessenger& messenger, size_t view_id) {
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
	});
	auto message = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	messenger.Send("zenith_subsurface_unmap", message->data(), message->size());
}
