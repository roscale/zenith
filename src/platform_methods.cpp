#include "platform_methods.hpp"
#include "input_callbacks.hpp"

extern "C" {
#define static
#include "wlr/types/wlr_seat.h"
#include "wlr/types/wlr_xdg_shell.h"
#undef static
}

void activate_window(ZenithOutput* output,
                     const flutter::MethodCall<> &call,
                     std::unique_ptr<flutter::MethodResult<>> result) {

	int64_t view_ptr_int = std::get<int64_t>(call.arguments()[0]);
	auto* view = reinterpret_cast<ZenithView*>(view_ptr_int);
	ZenithServer* server = output->server;

	bool view_doesnt_exist = server->views.find(view) == server->views.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}

	focus_view(view);

	result->Success();
}

void pointer_hover(ZenithOutput* output,
                   const flutter::MethodCall<> &call,
                   std::unique_ptr<flutter::MethodResult<>> result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);

	double x = std::get<double>(args[flutter::EncodableValue("x")]);
	double y = std::get<double>(args[flutter::EncodableValue("y")]);
	int64_t view_ptr_int = std::get<int64_t>(args[flutter::EncodableValue("view_ptr")]);

	auto* view = reinterpret_cast<ZenithView*>(view_ptr_int);
	ZenithServer* server = output->server;

	bool view_doesnt_exist = server->views.find(view) == server->views.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}

	wlr_seat_pointer_notify_enter(server->seat, view->xdg_surface->surface, x, y);
	wlr_seat_pointer_notify_motion(server->seat, FlutterEngineGetCurrentTime() / 1000000, x, y);
	// TODO: wlr_seat_pointer_clear_focus(seat); when hovering over another client.
	result->Success();
}

void close_window(ZenithOutput* output,
                  const flutter::MethodCall<> &call,
                  std::unique_ptr<flutter::MethodResult<>> result) {

	int64_t view_ptr_int = std::get<int64_t>(call.arguments()[0]);
	auto* view = reinterpret_cast<ZenithView*>(view_ptr_int);
	ZenithServer* server = output->server;

	bool view_doesnt_exist = server->views.find(view) == server->views.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}

	wlr_xdg_toplevel_send_close(view->xdg_surface);

	result->Success();
}