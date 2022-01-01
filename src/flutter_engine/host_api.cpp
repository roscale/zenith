#include "host_api.hpp"
#include "server.hpp"
#include "encodable_value.h"

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

void pointer_hover(ZenithOutput* output,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);

	double x = std::get<double>(args[flutter::EncodableValue("x")]);
	double y = std::get<double>(args[flutter::EncodableValue("y")]);
	size_t view_id = std::get<int>(args[flutter::EncodableValue("view_id")]);

	ZenithServer* server = output->server;

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
		wlr_seat_pointer_notify_motion(server->seat, FlutterEngineGetCurrentTime() / 1000000, sub_x, sub_y);
	} else {
		// This has to stay, otherwise down -> move -> up for selecting a popup entry doesn't work.
		wlr_seat_pointer_notify_enter(server->seat, view->xdg_surface->surface, x, y);
		wlr_seat_pointer_notify_motion(server->seat, FlutterEngineGetCurrentTime() / 1000000, x, y);
	}
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

void resize_window(ZenithOutput* output,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result) {

	flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
	size_t view_id = std::get<int>(args[flutter::EncodableValue("view_id")]);
	double width = std::get<double>(args[flutter::EncodableValue("width")]);
	double height = std::get<double>(args[flutter::EncodableValue("height")]);

	ZenithServer* server = output->server;

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