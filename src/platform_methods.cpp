#include "platform_methods.hpp"
#include "input_callbacks.hpp"

extern "C" {
#define static
#include "wlr/types/wlr_seat.h"
#include "wlr/types/wlr_xdg_shell.h"
#undef static
}

void activate_window(ZenithOutput* output,
                     const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result) {

	size_t view_id = std::get<int>(call.arguments()[0]);
	ZenithServer* server = output->server;

	auto view_it = server->views.find(view_id);
	bool view_doesnt_exist = view_it == server->views.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}
	ZenithView* view = view_it->second;

	focus_view(view);

	result->Success();
}

void pointer_hover(ZenithOutput* output,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);

	double x = std::get<double>(args[flutter::EncodableValue("x")]);
	double y = std::get<double>(args[flutter::EncodableValue("y")]);
	size_t view_id = std::get<int>(args[flutter::EncodableValue("view_id")]);

	ZenithServer* server = output->server;

	auto view_it = server->views.find(view_id);
	bool view_doesnt_exist = view_it == server->views.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}
	ZenithView* view = view_it->second;

	wlr_seat_pointer_notify_enter(server->seat, view->xdg_surface->surface, x, y);
	wlr_seat_pointer_notify_motion(server->seat, FlutterEngineGetCurrentTime() / 1000000, x, y);
	result->Success();
}

void pointer_exit(ZenithOutput* output,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	ZenithServer* server = output->server;

	wlr_seat_pointer_clear_focus(server->seat);
	result->Success();
}

void close_window(ZenithOutput* output,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result) {

	size_t view_id = std::get<int>(call.arguments()[0]);
	ZenithServer* server = output->server;

	auto view_it = server->views.find(view_id);
	bool view_doesnt_exist = view_it == server->views.end();
	if (view_doesnt_exist) {
		result->Success();
		return;
	}
	ZenithView* view = view_it->second;

	wlr_xdg_toplevel_send_close(view->xdg_surface);

	result->Success();
}