#include "view.hpp"
#include "server.hpp"
#include "platform_channels/encodable_value.h"
#include "standard_method_codec.h"

extern "C" {
#define static
#include <wlr/render/wlr_texture.h>
#undef static
}
//#include "platform_channels/encodable_value.h"
//#include "standard_method_codec.h"

static size_t next_view_id = 1;

ZenithView::ZenithView(ZenithServer* server, wlr_xdg_surface* xdg_surface)
	  : server(server), xdg_surface(xdg_surface), id(next_view_id++) {

	/* Listen to the various events it can emit */
	map.notify = xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &map);

	unmap.notify = xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &unmap);

	destroy.notify = xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &destroy);
}

void ZenithView::focus() {
	if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		// Popups cannot get focus.
		return;
	}

	wlr_seat* seat = server->seat;
	wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

	wlr_surface* prev_surface = seat->keyboard_state.focused_surface;

	bool is_surface_already_focused = prev_surface == xdg_surface->surface;
	if (is_surface_already_focused) {
		return;
	}

	if (prev_surface != nullptr) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		wlr_xdg_surface* previous = wlr_xdg_surface_from_wlr_surface(seat->keyboard_state.focused_surface);
		wlr_xdg_toplevel_set_activated(previous, false);
	}
	// Activate the new surface.
	wlr_xdg_toplevel_set_activated(xdg_surface, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_seat_keyboard_notify_enter(seat, xdg_surface->surface,
	                               keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

void xdg_surface_map(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, map);
	view->mapped = true;
	view->focus();

	std::cout << "ptr" << std::endl;
	std::cout << view->server->output->flutter_engine_state->engine << std::endl;

	wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);
	assert(texture != nullptr);


//	auto inserted_pair = server->surface_framebuffers.insert(
//		  std::pair<size_t, std::unique_ptr<SurfaceFramebuffer>>(
//				view->id,
//				std::make_unique<SurfaceFramebuffer>(texture->width, texture->height)
//		  )
//	);

	FlutterEngineRegisterExternalTexture(view->server->output->flutter_engine_state->engine, (int64_t) view->id);

	using namespace flutter;

	switch (view->xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			auto value = EncodableValue(EncodableMap{
				  {EncodableValue("view_id"),     EncodableValue((int64_t) view->id)},
				  {EncodableValue("surface_ptr"), EncodableValue((int64_t) view->xdg_surface->surface)},
				  {EncodableValue("width"),       EncodableValue(texture->width)},
				  {EncodableValue("height"),      EncodableValue(texture->height)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->flutter_engine_state->messenger.Send("window_mapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP: {
			wlr_xdg_popup* popup = view->xdg_surface->popup;
			auto value = EncodableValue(EncodableMap{
				  {EncodableValue("view_id"),            EncodableValue((int64_t) view->id)},
				  {EncodableValue("surface_ptr"),        EncodableValue((int64_t) view->xdg_surface->surface)},
				  {EncodableValue("parent_surface_ptr"), EncodableValue((int64_t) popup->parent)},
				  {EncodableValue("x"),                  EncodableValue(popup->geometry.x)},
				  {EncodableValue("y"),                  EncodableValue(popup->geometry.y)},
				  {EncodableValue("width"),              EncodableValue(popup->geometry.width)},
				  {EncodableValue("height"),             EncodableValue(popup->geometry.height)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->flutter_engine_state->messenger.Send("popup_mapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
	std::cout << "ptr" << std::endl;
	std::cout << view->server->output->flutter_engine_state->engine << std::endl;
}

void xdg_surface_unmap(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, unmap);
	view->mapped = false;

	using namespace flutter;

	switch (view->xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			auto value = EncodableValue(EncodableMap{
				  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->flutter_engine_state->messenger.Send("window_unmapped", result->data(),
			                                                           result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP: {
			wlr_xdg_popup* popup = view->xdg_surface->popup;

			auto value = EncodableValue(EncodableMap{
				  {EncodableValue("view_id"),            EncodableValue((int64_t) view->id)},
				  {EncodableValue("parent_surface_ptr"), EncodableValue((int64_t) popup->parent)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->flutter_engine_state->messenger.Send("popup_unmapped", result->data(),
			                                                           result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
}

void xdg_surface_destroy(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, destroy);

	view->server->views_by_id.erase(view->id);
}
