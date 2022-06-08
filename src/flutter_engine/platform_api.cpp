#include <linux/input-event-codes.h>
#include "platform_api.hpp"
#include "server.hpp"
#include "encodable_value.h"
#include "time.hpp"

extern "C" {
#define static
#include "wlr/types/wlr_seat.h"
#include "wlr/types/wlr_xdg_shell.h"
#include <wlr/render/gles2.h>
#undef static
}

void activate_window(ZenithServer* server,
                     const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result) {

	size_t view_id = std::get<int>(call.arguments()[0]);
	auto view_it = server->views_by_id.find(view_id);
	bool view_doesnt_exist = view_it == server->views_by_id.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}
	ZenithView* view = view_it->second.get();

	view->focus();

	result->Success();
}

void pointer_hover(ZenithServer* server,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);

	double x = std::get<double>(args[flutter::EncodableValue("x")]);
	double y = std::get<double>(args[flutter::EncodableValue("y")]);
	size_t view_id = std::get<int>(args[flutter::EncodableValue("view_id")]);

	auto view_it = server->views_by_id.find(view_id);
	bool view_doesnt_exist = view_it == server->views_by_id.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}
	ZenithView* view = view_it->second.get();

	double sub_x, sub_y;
	wlr_surface* leaf_surface = wlr_xdg_surface_surface_at(view->xdg_surface, x, y, &sub_x, &sub_y);
	if (leaf_surface == nullptr) {
		result->Success();
		return;
	}

	if (!wlr_surface_is_xdg_surface(leaf_surface)) {
		// Give pointer focus to an inner subsurface, if one exists.
		// This fixes GTK popovers.
		wlr_seat_pointer_notify_enter(server->seat, leaf_surface, sub_x, sub_y);
		wlr_seat_pointer_notify_motion(server->seat, current_time_milliseconds(), sub_x, sub_y);
	} else {
		// This has to stay, otherwise down -> move -> up for selecting a popup entry doesn't work.
		wlr_seat_pointer_notify_enter(server->seat, view->xdg_surface->surface, x, y);
		wlr_seat_pointer_notify_motion(server->seat, current_time_milliseconds(), x, y);
	}
	result->Success();
}


void pointer_exit(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {
	wlr_seat_pointer_notify_clear_focus(server->seat);
	wlr_xcursor_manager_set_cursor_image(server->pointer->cursor_mgr, "left_ptr", server->pointer->cursor);
	result->Success();
}

void close_window(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	size_t view_id = std::get<int>(call.arguments()[0]);

	auto view_it = server->views_by_id.find(view_id);
	bool view_doesnt_exist = view_it == server->views_by_id.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}
	ZenithView* view = view_it->second.get();

	wlr_xdg_toplevel_send_close(view->xdg_surface);

	result->Success();
}

void resize_window(ZenithServer* server,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = std::get<int>(args[flutter::EncodableValue("view_id")]);
	double width = std::get<double>(args[flutter::EncodableValue("width")]);
	double height = std::get<double>(args[flutter::EncodableValue("height")]);

	auto view_it = server->views_by_id.find(view_id);
	bool view_doesnt_exist = view_it == server->views_by_id.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}
	ZenithView* view = view_it->second.get();

	wlr_xdg_toplevel_set_size(view->xdg_surface, (size_t) width, (size_t) height);

	result->Success();
}

void closing_animation_finished(ZenithServer* server,
                                const flutter::MethodCall<>& call,
                                std::unique_ptr<flutter::MethodResult<>>&& result) {

	size_t view_id = std::get<int>(call.arguments()[0]);

	if (eglGetCurrentContext() == nullptr) {
		// wlroots is screwing with me and deactivates the gl context for some reason.
		wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
		wlr_egl_make_current(egl);
	}

	std::scoped_lock lock(server->surface_framebuffers_mutex);
	server->surface_framebuffers.erase(view_id);

	FlutterEngineUnregisterExternalTexture(server->flutter_engine_state->engine, view_id);

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
	result->Success();
}

