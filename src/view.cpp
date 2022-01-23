#include "view.hpp"
#include "server.hpp"
#include "platform_channels/encodable_value.h"
#include "standard_method_codec.h"

extern "C" {
#define static
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/gles2.h>
#undef static
}

using namespace flutter;

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

	commit.notify = surface_commit;
	wl_signal_add(&xdg_surface->surface->events.commit, &commit);

	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		request_move.notify = xdg_toplevel_request_move;
		wl_signal_add(&xdg_surface->toplevel->events.request_move, &request_move);

		request_resize.notify = xdg_toplevel_request_resize;
		wl_signal_add(&xdg_surface->toplevel->events.request_resize, &request_resize);
	}
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
		wlr_xdg_surface* previous;
		if (wlr_surface_is_xdg_surface(prev_surface)
		    && (previous = wlr_xdg_surface_from_wlr_surface(prev_surface)) != nullptr) {
			// FIXME: There is some weirdness going on which requires this seemingly redundant check.
			// I think the surface might be already destroyed but in this case keyboard_state.focused_surface
			// should be automatically set to null according to wlroots source code.
			// It seems that it doesn't cause any more crashes but I don't think this is the right fix.
			wlr_xdg_toplevel_set_activated(previous, false);
		}
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
	ZenithServer* server = view->server;
	view->mapped = true;
	view->focus();

	wlr_xdg_surface* xdg_surface = view->xdg_surface;
	wlr_surface* surface = xdg_surface->surface;
	wlr_texture* texture = wlr_surface_get_texture(surface);
	assert(texture != nullptr);

	// Make sure a framebuffer exists for this xdg_surface.
	wlr_egl_make_current(wlr_gles2_renderer_get_egl(server->renderer));

	{
		std::scoped_lock lock(server->surface_framebuffers_mutex);

		auto surface_framebuffer_it = server->surface_framebuffers.find(view->id);
		if (surface_framebuffer_it == server->surface_framebuffers.end()) {
			server->surface_framebuffers.insert(
				  std::pair(
						view->id,
						std::make_shared<SurfaceFramebuffer>(texture->width, texture->height)
				  )
			);
		}
	}

	FlutterEngineRegisterExternalTexture(view->server->flutter_engine_state->engine, (int64_t) view->id);

	switch (view->xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			auto value = EncodableValue(EncodableMap{
				  {EncodableValue("view_id"),        EncodableValue((int64_t) view->id)},
				  {EncodableValue("surface_width"),  EncodableValue(surface->current.width)},
				  {EncodableValue("surface_height"), EncodableValue(surface->current.height)},
				  {EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
						{EncodableValue("x"),      EncodableValue(xdg_surface->geometry.x)},
						{EncodableValue("y"),      EncodableValue(xdg_surface->geometry.y)},
						{EncodableValue("width"),  EncodableValue(xdg_surface->geometry.width)},
						{EncodableValue("height"), EncodableValue(xdg_surface->geometry.height)},
				  })},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->flutter_engine_state->messenger.Send("window_mapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP: {
			wlr_xdg_popup* popup = view->xdg_surface->popup;
			view->x = popup->geometry.x;
			view->y = popup->geometry.y;

			wlr_xdg_surface* parent_xdg_surface = wlr_xdg_surface_from_wlr_surface(popup->parent);
//			wlr_box parent_geometry = parent_xdg_surface->geometry;
			size_t parent_view_id = view->server->view_id_by_wlr_surface[parent_xdg_surface->surface];

			auto value = EncodableValue(EncodableMap{
				  {EncodableValue("view_id"),        EncodableValue((int64_t) view->id)},
				  {EncodableValue("parent_view_id"), EncodableValue((int64_t) parent_view_id)},
				  {EncodableValue("x"),              EncodableValue(popup->geometry.x)},
				  {EncodableValue("y"),              EncodableValue(popup->geometry.y)},
				  {EncodableValue("surface_width"),  EncodableValue(view->xdg_surface->surface->current.width)},
				  {EncodableValue("surface_height"), EncodableValue(view->xdg_surface->surface->current.height)},
				  {EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
						{EncodableValue("x"),      EncodableValue(popup->base->geometry.x)},
						{EncodableValue("y"),      EncodableValue(popup->base->geometry.y)},
						{EncodableValue("width"),  EncodableValue(popup->base->geometry.width)},
						{EncodableValue("height"), EncodableValue(popup->base->geometry.height)},
				  })},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->flutter_engine_state->messenger.Send("popup_mapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
}

void xdg_surface_unmap(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, unmap);
	view->mapped = false;

	switch (view->xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			auto value = EncodableValue(EncodableMap{
				  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->flutter_engine_state->messenger.Send("window_unmapped", result->data(),
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

			view->server->flutter_engine_state->messenger.Send("popup_unmapped", result->data(),
			                                                           result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
}

void xdg_surface_destroy(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, destroy);
	ZenithServer* server = view->server;

	wl_list_remove(&view->commit.link);

	size_t erased = server->view_id_by_wlr_surface.erase(view->xdg_surface->surface);
	assert(erased == 1);
	erased = server->views_by_id.erase(view->id);
	assert(erased == 1);
}

void surface_commit(wl_listener* listener, void* data) {
	auto* surface = static_cast<wlr_surface*>(data);
	auto* server = ZenithServer::instance();

	auto it_id = server->view_id_by_wlr_surface.find(surface);
	assert(it_id != server->view_id_by_wlr_surface.end());

	auto* view = server->views_by_id.find(it_id->second)->second.get();
	wlr_xdg_surface* xdg_surface = view->xdg_surface;

	auto map = EncodableMap{
		  {EncodableValue("view_id"),        EncodableValue((int64_t) view->id)},
		  {EncodableValue("surface_role"),   EncodableValue(xdg_surface->role)},
		  {EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
				{EncodableValue("x"),      EncodableValue(xdg_surface->geometry.x)},
				{EncodableValue("y"),      EncodableValue(xdg_surface->geometry.y)},
				{EncodableValue("width"),  EncodableValue(xdg_surface->geometry.width)},
				{EncodableValue("height"), EncodableValue(xdg_surface->geometry.height)},
		  })}
	};

	wlr_box new_geometry = xdg_surface->geometry;
	bool geometry_changed = false;
	if (view->geometry.x != new_geometry.x
	    or view->geometry.y != new_geometry.y
	    or view->geometry.width != new_geometry.width
	    or view->geometry.height != new_geometry.height) {

		geometry_changed = true;
		view->geometry = new_geometry;
		map.insert({EncodableValue("geometry_changed"), EncodableValue(true)});
		map.insert({EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
			  {EncodableValue("x"),      EncodableValue(xdg_surface->geometry.x)},
			  {EncodableValue("y"),      EncodableValue(xdg_surface->geometry.y)},
			  {EncodableValue("width"),  EncodableValue(xdg_surface->geometry.width)},
			  {EncodableValue("height"), EncodableValue(xdg_surface->geometry.height)},
		})});
	} else {
		map.insert({EncodableValue("geometry_changed"), EncodableValue(false)});
	}

	std::shared_ptr<SurfaceFramebuffer> surface_framebuffer;
	{
		std::scoped_lock lock(server->surface_framebuffers_mutex);

		auto it = server->surface_framebuffers.find(view->id);
		bool framebuffer_doesnt_exist = it == server->surface_framebuffers.end();
		if (framebuffer_doesnt_exist) {
			return;
		}

		surface_framebuffer = it->second;
	}

	std::scoped_lock lock(surface_framebuffer->mutex);

	bool surface_size_changed = false;
	if ((size_t) surface->current.buffer_width != surface_framebuffer->pending_width
	    or (size_t) surface->current.buffer_height != surface_framebuffer->pending_height) {

		surface_size_changed = true;
		// The actual resizing is happening on a Flutter thread because resizing a texture is very slow, and I don't want
		// to block the main thread causing input delay and other stuff.
		surface_framebuffer->schedule_resize(surface->current.buffer_width, surface->current.buffer_height);
		map.insert({EncodableValue("surface_size_changed"), EncodableValue(true)});
		map.insert({EncodableValue("surface_width"), EncodableValue((int64_t) surface->current.buffer_width)});
		map.insert({EncodableValue("surface_height"), EncodableValue((int64_t) surface->current.buffer_height)});
	} else {
		map.insert({EncodableValue("surface_size_changed"), EncodableValue(false)});
	}

	bool popup_position_changed = false;
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
		if (xdg_surface->popup->geometry.x != view->x
		    or xdg_surface->popup->geometry.y != view->y) {
			view->x = xdg_surface->popup->geometry.x;
			view->y = xdg_surface->popup->geometry.y;

			std::cout << "position changed" << std::endl;

			popup_position_changed = true;
			map.insert({EncodableValue("popup_position_changed"), EncodableValue(true)});
			map.insert({EncodableValue("x"), EncodableValue(xdg_surface->popup->geometry.x)});
			map.insert({EncodableValue("y"), EncodableValue(xdg_surface->popup->geometry.y)});
		} else {
			map.insert({EncodableValue("popup_position_changed"), EncodableValue(false)});
		}
	}

	if (geometry_changed or surface_size_changed or popup_position_changed) {
		auto value = EncodableValue(map);

		auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
		view->server->flutter_engine_state->messenger.Send("configure_surface", result->data(),
		                                                           result->size());
	}
}

void xdg_toplevel_request_move(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, request_move);

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->flutter_engine_state->messenger.Send("request_move", result->data(),
	                                                           result->size());
}

void xdg_toplevel_request_resize(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, request_resize);
	auto* event = static_cast<wlr_xdg_toplevel_resize_event*>(data);

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
		  {EncodableValue("edges"),   EncodableValue(event->edges)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->flutter_engine_state->messenger.Send("request_resize", result->data(),
	                                                           result->size());
}