#include "wayland_view.hpp"
#include "server.hpp"
#include <cassert>

#include "platform_channels/encodable_value.h"
#include "standard_method_codec.h"
#include "util.hpp"

extern "C" {
#include <wlr/render/gles2.h>
}

using namespace flutter;

ZenithWaylandView::ZenithWaylandView(ZenithServer* server, wlr_xdg_surface* xdg_surface)
	  : ZenithView(server),
	    xdg_surface(xdg_surface) {

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

void xdg_surface_map(wl_listener* listener, void* data) {
	ZenithWaylandView* view = wl_container_of(listener, view, map);
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
			view->popup_geometry = popup->geometry;

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
	ZenithWaylandView* view = wl_container_of(listener, view, unmap);
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
	ZenithWaylandView* view = wl_container_of(listener, view, destroy);
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
	auto* wayland_view = dynamic_cast<ZenithWaylandView*>(view);

	wlr_xdg_surface* xdg_surface = wayland_view->xdg_surface;

	auto map = EncodableMap{
		  {EncodableValue("view_id"),        EncodableValue((int64_t) wayland_view->id)},
		  {EncodableValue("surface_role"),   EncodableValue(xdg_surface->role)},
	};

	bool geometry_changed = not wlr_box_equal(xdg_surface->geometry, wayland_view->geometry);

	map.insert({EncodableValue("geometry_changed"), EncodableValue(geometry_changed)});

	if (geometry_changed) {
		wlr_box& new_geometry = xdg_surface->geometry;
		wayland_view->geometry = new_geometry;

		map.insert({EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
			  {EncodableValue("x"),      EncodableValue(new_geometry.x)},
			  {EncodableValue("y"),      EncodableValue(new_geometry.y)},
			  {EncodableValue("width"),  EncodableValue(new_geometry.width)},
			  {EncodableValue("height"), EncodableValue(new_geometry.height)},
		})});
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

	bool surface_size_changed;
	{
		std::scoped_lock lock(surface_framebuffer->mutex);

		surface_size_changed =
			  (size_t) surface->current.buffer_width != surface_framebuffer->pending_width or
			  (size_t) surface->current.buffer_height != surface_framebuffer->pending_height;

		map.insert({EncodableValue("surface_size_changed"), EncodableValue(surface_size_changed)});

		if (surface_size_changed) {
			int& new_surface_width = surface->current.buffer_width;
			int& new_surface_height = surface->current.buffer_height;
			// The actual resizing is happening on a Flutter thread because resizing a texture is very slow, and I don't want
			// to block the main thread causing input delay and other stuff.
			surface_framebuffer->schedule_resize(new_surface_width, new_surface_height);

			map.insert({EncodableValue("surface_width"), EncodableValue((int64_t) new_surface_width)});
			map.insert({EncodableValue("surface_height"), EncodableValue((int64_t) new_surface_height)});
		}
	}

	bool is_popup = xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP;
	bool popup_position_changed =
		  is_popup and
		  wlr_box_equal(xdg_surface->popup->geometry, wayland_view->popup_geometry);

	if (is_popup) {
		map.insert({EncodableValue("popup_position_changed"), EncodableValue(popup_position_changed)});

		if (popup_position_changed) {
			wayland_view->popup_geometry = xdg_surface->popup->geometry;

			map.insert({EncodableValue("x"), EncodableValue(xdg_surface->popup->geometry.x)});
			map.insert({EncodableValue("y"), EncodableValue(xdg_surface->popup->geometry.y)});
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
	ZenithWaylandView* view = wl_container_of(listener, view, request_move);

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->flutter_engine_state->messenger.Send("request_move", result->data(),
	                                                   result->size());
}

void xdg_toplevel_request_resize(wl_listener* listener, void* data) {
	ZenithWaylandView* view = wl_container_of(listener, view, request_resize);
	auto* event = static_cast<wlr_xdg_toplevel_resize_event*>(data);

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
		  {EncodableValue("edges"),   EncodableValue(event->edges)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->flutter_engine_state->messenger.Send("request_resize", result->data(),
	                                                   result->size());
}

void ZenithWaylandView::focus() {
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

void ZenithWaylandView::close() {
	wlr_xdg_toplevel_send_close(xdg_surface);
}

void ZenithWaylandView::pointer_hover(double x, double y) {
	double sub_x, sub_y;
	wlr_surface* leaf_surface = wlr_xdg_surface_surface_at(xdg_surface, x, y, &sub_x, &sub_y);
	if (leaf_surface == nullptr) {
		return;
	}

	if (!wlr_surface_is_xdg_surface(leaf_surface)) {
		// Give pointer focus to an inner subsurface, if one exists.
		// This fixes GTK popovers.
		wlr_seat_pointer_notify_enter(server->seat, leaf_surface, sub_x, sub_y);
		wlr_seat_pointer_notify_motion(server->seat, FlutterEngineGetCurrentTime() / 1000000, sub_x, sub_y);
	} else {
		// This has to stay, otherwise down -> move -> up for selecting a popup entry doesn't work.
		wlr_seat_pointer_notify_enter(server->seat, xdg_surface->surface, x, y);
		wlr_seat_pointer_notify_motion(server->seat, FlutterEngineGetCurrentTime() / 1000000, x, y);
	}
}

void ZenithWaylandView::resize(uint32_t width, uint32_t height) {
	wlr_xdg_toplevel_set_size(xdg_surface, (size_t) width, (size_t) height);
}

void ZenithWaylandView::render_to_fbo(GLuint fbo) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	render_data<ZenithWaylandView> rdata = {
		  .view = this,
		  .view_fbo = fbo,
	};

	try {
		// Render the subtree of subsurfaces starting from a toplevel or popup.
		wlr_xdg_surface_for_each_surface(
			  xdg_surface,
			  [](struct wlr_surface* surface, int sx, int sy, void* data) {
				  auto* rdata = static_cast<render_data<ZenithWaylandView>*>(data);
				  auto* view = rdata->view;

				  if (surface != view->xdg_surface->surface && wlr_surface_is_xdg_surface(surface)) {
					  // Don't render child popups. They will be rendered separately on their own framebuffer,
					  // so skip this subtree of surfaces.
					  // There's no easy way to exit this recursive iterator, but we can still do this by throwing a
					  // dummy exception.
					  throw std::exception();
				  }

				  wlr_texture* texture = wlr_surface_get_texture(surface);
				  if (texture == nullptr) {
					  // I don't remember if it's possible to have a mapped surface without texture, but better to be
					  // safe than sorry. Just skip it.
					  return;
				  }

				  wlr_gles2_texture_attribs attribs{};
				  wlr_gles2_texture_get_attribs(texture, &attribs);

				  if (wlr_surface_is_xdg_surface(surface)) {
					  // xdg_surfaces each have their own framebuffer, so they are always drawn at (0, 0).
					  RenderToTextureShader::instance()->render(attribs.tex, 0, 0,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_fbo);
				  } else {
					  // This is true when the surface is a wayland subsurface and not an xdg_surface.
					  // I decided to render subsurfaces on the framebuffer of the nearest xdg_surface parent.
					  // One downside to this approach is that popups created using a subsurface instead of an xdg_popup
					  // are clipped to the window and cannot be rendered outside the window bounds.
					  // I don't really care because Gnome Shell takes the same approach, and it simplifies things a lot.
					  // Interesting discussion: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/24
					  RenderToTextureShader::instance()->render(attribs.tex, sx, sy,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_fbo);
				  }

				  // Tell the client we are done rendering this surface.
				  timespec now{};
				  clock_gettime(CLOCK_MONOTONIC, &now);
				  wlr_surface_send_frame_done(surface, &now);
			  },
			  &rdata
		);
	} catch (std::exception& unused) {
	}
}
