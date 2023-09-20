#include <linux/input-event-codes.h>
#include <csignal>
#include "platform_api.hpp"
#include "server.hpp"
#include "encodable_value.h"
#include "time.hpp"
#include "util/embedder/keyboard_helpers.hpp"
#include "assert.hpp"
#include "util/wlr/wlr_extensions.hpp"
#include "auth.hpp"

extern "C" {
#define static
#include "wlr/types/wlr_seat.h"
#include "wlr/types/wlr_xdg_shell.h"
#undef static
}

void startup_complete(ZenithServer* server, const flutter::MethodCall<>& call,
                      std::unique_ptr<flutter::MethodResult<>>&& result) {

	server->callable_queue.enqueue([server] {
		if (!server->startup_command.empty()) {
			if (fork() == 0) {
				execl("/bin/sh", "/bin/sh", "-c", server->startup_command.c_str(), nullptr);
			}
		}
	});
	result->Success();
}

void activate_window(ZenithServer* server,
                     const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result) {

	auto list = std::get<flutter::EncodableList>(call.arguments()[0]);
	size_t view_id = list[0].LongValue();
	bool activate = std::get<bool>(list[1]);

	server->callable_queue.enqueue([server, view_id, activate] {
		auto view_it = server->xdg_toplevels.find(view_id);
		if (view_it == server->xdg_toplevels.end()) {
			return;
		}
		auto* view = view_it->second.get();
		view->focus(activate);
	});

	result->Success();
}

void pointer_hover(ZenithServer* server,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);

	double x = std::get<double>(args[flutter::EncodableValue("x")]);
	double y = std::get<double>(args[flutter::EncodableValue("y")]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();

	server->callable_queue.enqueue([server, x, y, view_id] {
		auto view_it = server->surfaces.find(view_id);
		if (view_it == server->surfaces.end()) {
			return;
		}
		ZenithSurface* view = view_it->second.get();

		wlr_seat_pointer_notify_enter(server->seat, view->surface, x, y);
		wlr_seat_pointer_notify_motion(server->seat, current_time_milliseconds(), x, y);
	});
	result->Success();
}

void pointer_exit(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	server->callable_queue.enqueue([server] {
		wlr_seat_pointer_notify_clear_focus(server->seat);
		if (server->pointer != nullptr && server->pointer->is_visible()) {
			wlr_xcursor_manager_set_cursor_image(server->pointer->cursor_mgr, "left_ptr", server->pointer->cursor);
		}
	});
	result->Success();
}

void close_window(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();

	server->callable_queue.enqueue([server, view_id] {
		auto view_it = server->xdg_toplevels.find(view_id);
		if (view_it == server->xdg_toplevels.end()) {
			return;
		}
		ZenithXdgToplevel* view = view_it->second.get();
		wlr_xdg_toplevel_send_close(view->xdg_toplevel->base);
	});
	result->Success();
}

void resize_window(ZenithServer* server,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();
	auto width = std::get<int>(args[flutter::EncodableValue("width")]);
	auto height = std::get<int>(args[flutter::EncodableValue("height")]);

	server->callable_queue.enqueue([server, view_id, width, height] {
		auto view_it = server->xdg_toplevels.find(view_id);
		if (view_it == server->xdg_toplevels.end()) {
			return;
		}
		ZenithXdgToplevel* view = view_it->second.get();
		wlr_xdg_toplevel_set_size(view->xdg_toplevel->base, (uint32_t) width, (uint32_t) height);
	});

	result->Success();
}

void maximize_window(ZenithServer* server, const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result) {
	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();
	auto value = std::get<bool>(args[flutter::EncodableValue("value")]);

	server->callable_queue.enqueue([server, view_id, value] {
		auto view_it = server->xdg_toplevels.find(view_id);
		if (view_it == server->xdg_toplevels.end()) {
			return;
		}
		ZenithXdgToplevel* view = view_it->second.get();
		view->maximize(value);
	});

	result->Success();
}

void unregister_view_texture(ZenithServer* server,
                             const flutter::MethodCall<>& call,
                             std::unique_ptr<flutter::MethodResult<>>&& result) {

	size_t texture_id = call.arguments()[0].LongValue();
	server->callable_queue.enqueue([server, texture_id] {
		assert(server->surface_id_per_texture_id.count(texture_id) == 1);
		server->surface_buffer_chains.erase(texture_id);

		size_t view_id = server->surface_id_per_texture_id.at(texture_id);
		server->surface_id_per_texture_id.erase(texture_id);

		auto& texture_ids = server->texture_ids_per_surface_id.at(view_id);
		auto it = std::find(texture_ids.begin(), texture_ids.end(), texture_id);
		assert(it != texture_ids.end());
		texture_ids.erase(it);

		if (texture_ids.empty()) {
			server->texture_ids_per_surface_id.erase(view_id);
		}
	});

	result->Success();
}

void mouse_button_event(ZenithServer* server, const flutter::MethodCall<>& call,
                        std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	auto is_pressed = std::get<bool>(args[flutter::EncodableValue("is_pressed")]);
	auto button = std::get<int>(args[flutter::EncodableValue("button")]);

	int32_t linux_button = 0;
	switch (button) {
		case kFlutterPointerButtonMousePrimary:
			linux_button = BTN_LEFT;
			break;
		case kFlutterPointerButtonMouseSecondary:
			linux_button = BTN_RIGHT;
			break;
		case kFlutterPointerButtonMouseMiddle:
			linux_button = BTN_MIDDLE;
			break;
		default:
			break;
	}

	server->callable_queue.enqueue([server, linux_button, is_pressed] {
		wlr_seat_pointer_notify_button(server->seat, current_time_milliseconds(), linux_button,
		                               is_pressed ? WLR_BUTTON_PRESSED : WLR_BUTTON_RELEASED);
		// FIXME:
		// We should not manually trigger a frame event because one is received automatically by ZenithPointer at the right
		// time. However, by the time this frame event is received, the button event has not finished routing through
		// Flutter and back to the platform, therefore, it will be sent too early and their execution order is swapped.
		// The frame event has to come after the button event, so call it manually here.
		// There are now 2 frame events generated per button event, one before, and one after. It doesn't seem to cause any
		// problems in the applications that I tested though.
		// In any case, pointer support is considered a second-class citizen as this desktop environment is targeted towards
		// mobile devices first and foremost, but the pointer is still useful during development.
		wlr_seat_pointer_notify_frame(server->seat);
	});
	result->Success();
}

void change_window_visibility(ZenithServer* server, const flutter::MethodCall<>& call,
                              std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();
	auto visible = std::get<bool>(args[flutter::EncodableValue("visible")]);

	server->callable_queue.enqueue([server, view_id, visible] {
		auto view_it = server->xdg_toplevels.find(view_id);
		if (view_it == server->xdg_toplevels.end()) {
			return;
		}
		ZenithXdgToplevel* view = view_it->second.get();
		view->visible = visible;
	});
	result->Success();
}

void touch_down(ZenithServer* server, const flutter::MethodCall<>& call,
                std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();
	auto touch_id = std::get<int>(args[flutter::EncodableValue("touch_id")]);
	auto x = std::get<double>(args[flutter::EncodableValue("x")]);
	auto y = std::get<double>(args[flutter::EncodableValue("y")]);

	server->callable_queue.enqueue([server, view_id, touch_id, x, y] {
		auto view_it = server->surfaces.find(view_id);
		if (view_it == server->surfaces.end()) {
			return;
		}
		ZenithSurface* view = view_it->second.get();

		wlr_seat_touch_notify_down(server->seat, view->surface, current_time_milliseconds(), touch_id, x, y);
		wlr_seat_touch_notify_frame(server->seat);
	});

	result->Success();
}

void touch_motion(ZenithServer* server, const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	auto touch_id = std::get<int>(args[flutter::EncodableValue("touch_id")]);
	auto x = std::get<double>(args[flutter::EncodableValue("x")]);
	auto y = std::get<double>(args[flutter::EncodableValue("y")]);

	server->callable_queue.enqueue([server, touch_id, x, y] {
		wlr_seat_touch_notify_motion(server->seat, current_time_milliseconds(), touch_id, x, y);
		wlr_seat_touch_notify_frame(server->seat);
	});

	result->Success();
}

void touch_up(ZenithServer* server, const flutter::MethodCall<>& call,
              std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	auto touch_id = std::get<int>(args[flutter::EncodableValue("touch_id")]);

	server->callable_queue.enqueue([server, touch_id] {
		wlr_seat_touch_notify_up(server->seat, current_time_milliseconds(), touch_id);
		wlr_seat_touch_notify_frame(server->seat);
	});

	result->Success();
}

void touch_cancel(ZenithServer* server, const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	auto touch_id = std::get<int>(args[flutter::EncodableValue("touch_id")]);

	server->callable_queue.enqueue([server, touch_id] {
		wlr_touch_point* point = wlr_seat_touch_get_point(server->seat, touch_id);
		if (point == nullptr) {
			return;
		}
		struct wlr_surface* surface = point->surface;
		if (surface == nullptr) {
			return;
		}

		zenith::wlr_seat_touch_notify_cancel(server->seat, surface);
		wlr_seat_touch_notify_frame(server->seat);
	});

	result->Success();
}

void insert_text(ZenithServer* server, const flutter::MethodCall<>& call,
                 std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();
	auto text = std::get<std::string>(args[flutter::EncodableValue("text")]);

	if (view_id == 0) {
		// Text field focused in the compositor, not an application.
		server->embedder_state->callable_queue.enqueue([server, text] {
			auto& client = server->embedder_state->current_text_input_client;
			if (client.has_value()) {
				client->add_text(text);
			}
		});
		result->Success();
		return;
	}

	server->callable_queue.enqueue([server, view_id, text = std::move(text)] {
		auto view_it = server->surfaces.find(view_id);
		if (view_it == server->surfaces.end()) {
			return;
		}
		ZenithSurface* view = view_it->second.get();
		if (view->active_text_input != nullptr) {
			wlr_text_input_v3_send_commit_string(view->active_text_input->wlr_text_input, text.c_str());
			wlr_text_input_v3_send_done(view->active_text_input->wlr_text_input);
		} else {
			wlr_keyboard* keyboard = wlr_seat_get_keyboard(server->seat);
			if (keyboard == nullptr) {
				return;
			}

			std::optional<KeyCombination> combination = string_to_keycode(text.c_str(), keyboard->xkb_state);

			ASSERT(combination.has_value(),
			       "Character " << text << " cannot be emulated with the current keyboard layout.");

			if (combination) {
				auto mods = wlr_keyboard_modifiers{
					  .depressed = combination->modifiers,
					  .latched = 0,
					  .locked = 0,
					  .group = 0,
				};
				auto previous_mods = keyboard->modifiers;

				wlr_seat_keyboard_notify_modifiers(server->seat, &mods);
				wlr_seat_keyboard_notify_key(server->seat, current_time_milliseconds(), combination->keycode - 8,
				                             WL_KEYBOARD_KEY_STATE_PRESSED);
				wlr_seat_keyboard_notify_key(server->seat, current_time_milliseconds(), combination->keycode - 8,
				                             WL_KEYBOARD_KEY_STATE_RELEASED);
				wlr_seat_keyboard_notify_modifiers(server->seat, &previous_mods);
			}
		}
	});

	result->Success();
}

void emulate_keycode(ZenithServer* server, const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();
	auto keycode = std::get<int>(args[flutter::EncodableValue("keycode")]);

	if (view_id == 0) {
		// Text field focused in the compositor, not an application.
		server->embedder_state->callable_queue.enqueue([server, keycode] {
			auto& embedder_state = server->embedder_state;
			auto& client = embedder_state->current_text_input_client;
			if (client.has_value()) {
				switch (keycode) {
					case 14:
						client->backspace();
						break;
					case 28: {
						client->enter();
						break;
					}
					default:
						break;
				}
				embedder_state->update_text_editing_state();
			}
		});

		result->Success();
		return;
	}

	server->callable_queue.enqueue([server, keycode] {
		wlr_keyboard* keyboard = wlr_seat_get_keyboard(server->seat);
		if (keyboard == nullptr) {
			return;
		}

		wlr_seat_keyboard_notify_key(server->seat, current_time_milliseconds(), keycode,
		                             WL_KEYBOARD_KEY_STATE_PRESSED);
		wlr_seat_keyboard_notify_key(server->seat, current_time_milliseconds(), keycode,
		                             WL_KEYBOARD_KEY_STATE_RELEASED);
	});

	result->Success();
}

void open_windows_maximized(ZenithServer* server, const flutter::MethodCall<>& call,
                             std::unique_ptr<flutter::MethodResult<>>&& result) {
	auto value = std::get<bool>(call.arguments()[0]);

	server->callable_queue.enqueue([server, value] {
		server->open_windows_maximized = value;
	});

	result->Success();
}

void maximized_window_size(ZenithServer* server, const flutter::MethodCall<>& call,
                           std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	auto width = std::get<int>(args[flutter::EncodableValue("width")]);
	auto height = std::get<int>(args[flutter::EncodableValue("height")]);

	ASSERT(width >= 0, "width must be >= 0");
	ASSERT(height >= 0, "height must be >= 0");

	server->callable_queue.enqueue([server, width, height] {
		server->max_window_size = Size{
			  .width = static_cast<uint32_t>(width),
			  .height = static_cast<uint32_t>(height),
		};
	});

	result->Success();
}

void unlock_session(ZenithServer* server, const flutter::MethodCall<>& call,
                    std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	auto password = std::get<std::string>(args[flutter::EncodableValue("password")]);

	AuthenticationResponse response = authenticate_current_user(password);

	result->Success(flutter::EncodableValue{
		  flutter::EncodableMap{
				{flutter::EncodableValue("success"), flutter::EncodableValue(response.success)},
				{flutter::EncodableValue("message"), flutter::EncodableValue(response.message)},
		  }
	});
}

void enable_display(ZenithServer* server, const flutter::MethodCall<>& call,
                    std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	auto enable = std::get<bool>(args[flutter::EncodableValue("enable")]);

	server->callable_queue.enqueue([server, enable] {
		if (server->output == nullptr) {
			return;
		}
		if (enable) {
			server->output->enable();
		} else {
			server->output->disable();
		}
	});

	result->Success();
}

void hide_keyboard(ZenithServer* server, const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = args[flutter::EncodableValue("view_id")].LongValue();

	server->embedder_state->callable_queue.enqueue([=]() {
		auto& client = server->embedder_state->current_text_input_client;
		if (view_id == 0 && client.has_value()) {
			client->close_connection();
		}
	});

	result->Success();
}
