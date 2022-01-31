#include "host_api.hpp"
#include "server.hpp"
#include "encodable_value.h"

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

	view->pointer_hover(x, y);

	result->Success();
}


void pointer_exit(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	wlr_seat_pointer_clear_focus(server->seat);
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

	view->close();

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

	view->resize((uint32_t) width, (uint32_t) height);

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