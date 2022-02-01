#include "x11_view.hpp"
#include "server.hpp"
#include <cassert>
#include <standard_method_codec.h>

extern "C" {
#include <wlr/render/gles2.h>
}

using namespace flutter;

ZenithX11View::ZenithX11View(ZenithServer* server, wlr_xwayland_surface* x11_surface) : ZenithView(
	  server), x11_surface(x11_surface) {

	map.notify = xwayland_surface_map;
	wl_signal_add(&x11_surface->events.map, &map);

	unmap.notify = xwayland_surface_unmap;
	wl_signal_add(&x11_surface->events.unmap, &unmap);

	destroy.notify = xwayland_surface_destroy;
	wl_signal_add(&x11_surface->events.destroy, &destroy);

	request_move.notify = xwayland_surface_request_move;
	wl_signal_add(&x11_surface->events.request_move, &request_move);

	request_resize.notify = xwayland_surface_request_resize;
	wl_signal_add(&x11_surface->events.request_resize, &request_resize);

	set_geometry.notify = xwayland_surface_set_geometry;
	wl_signal_add(&x11_surface->events.set_geometry, &set_geometry);

//	commit.notify = surface_commit;
//	wl_signal_add(&x11_surface->surface->events.commit, &commit);
}

void ZenithX11View::focus() {
	wlr_seat* seat = server->seat;
	wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

	wlr_surface* prev_surface = seat->keyboard_state.focused_surface;

	bool is_surface_already_focused = prev_surface == x11_surface->surface;
	if (is_surface_already_focused) {
		return;
	}

//	if (prev_surface != nullptr) {
//		/*
//		 * Deactivate the previously focused surface. This lets the client know
//		 * it no longer has focus and the client will repaint accordingly, e.g.
//		 * stop displaying a caret.
//		 */
//		wlr_xdg_surface* previous;
//		if (wlr_surface_is_xdg_surface(prev_surface)
//		    && (previous = wlr_xdg_surface_from_wlr_surface(prev_surface)) != nullptr) {
//			// FIXME: There is some weirdness going on which requires this seemingly redundant check.
//			// I think the surface might be already destroyed but in this case keyboard_state.focused_surface
//			// should be automatically set to null according to wlroots source code.
//			// It seems that it doesn't cause any more crashes but I don't think this is the right fix.
//			wlr_xdg_toplevel_set_activated(previous, false);
//		}
//	}
	// Activate the new surface.
	wlr_xwayland_surface_activate(x11_surface, true);
//	wlr_xdg_toplevel_set_activated(xdg_surface, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_seat_keyboard_notify_enter(seat, x11_surface->surface,
	                               keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

void ZenithX11View::close() {

}

void ZenithX11View::pointer_hover(double x, double y) {
	if (x11_surface->surface == nullptr) {
		return;
	}
	wlr_seat_pointer_notify_enter(server->seat, x11_surface->surface, x, y);
	wlr_seat_pointer_notify_motion(server->seat, FlutterEngineGetCurrentTime() / 1'000'000, x, y);
}

void ZenithX11View::resize(uint32_t width, uint32_t height) {
	wlr_xwayland_surface_configure(x11_surface, 0, 0, (size_t) width, (size_t) height);
	xwayland_surface_set_geometry(&set_geometry, nullptr);
}

void ZenithX11View::render_to_fbo(GLuint fbo) {
	wlr_surface* surface = x11_surface->surface;
	wlr_texture* texture = wlr_surface_get_texture(surface);
	if (texture == nullptr) {
		// I don't remember if it's possible to have a mapped surface without texture, but better to be
		// safe than sorry. Just skip it.
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	wlr_gles2_texture_attribs attribs{};
	wlr_gles2_texture_get_attribs(texture, &attribs);

	RenderToTextureShader::instance()->render(attribs.tex, 0, 0,
	                                          surface->current.buffer_width,
	                                          surface->current.buffer_height,
	                                          fbo);

	// Tell the client we are done rendering this surface.
	timespec now{};
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_surface_send_frame_done(surface, &now);
}

void xwayland_surface_map(wl_listener* listener, void* data) {
	ZenithX11View* view = wl_container_of(listener, view, map);
	ZenithServer* server = view->server;
	view->mapped = true;
	std::cout << "MAP" << std::endl;
	view->focus();

	wlr_xwayland_surface* x11_surface = view->x11_surface;
	wlr_surface* surface = x11_surface->surface;
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

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"),        EncodableValue((int64_t) view->id)},
		  {EncodableValue("surface_width"),  EncodableValue(surface->current.width)},
		  {EncodableValue("surface_height"), EncodableValue(surface->current.height)},
		  {EncodableValue("visible_bounds"), EncodableValue(EncodableMap{
				{EncodableValue("x"),      EncodableValue(0)},
				{EncodableValue("y"),      EncodableValue(0)},
				{EncodableValue("width"),  EncodableValue(x11_surface->width)},
				{EncodableValue("height"), EncodableValue(x11_surface->height)},
		  })},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

	view->server->flutter_engine_state->messenger.Send("window_mapped", result->data(), result->size());
}

void xwayland_surface_unmap(wl_listener* listener, void* data) {
	ZenithX11View* view = wl_container_of(listener, view, unmap);
	view->mapped = false;

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

	view->server->flutter_engine_state->messenger.Send("window_unmapped", result->data(),
	                                                   result->size());
}

void xwayland_surface_destroy(wl_listener* listener, void* data) {
	ZenithX11View* view = wl_container_of(listener, view, destroy);
	ZenithServer* server = view->server;

//	wl_list_remove(&view->commit.link);

	size_t erased = server->view_id_by_wlr_surface.erase(view->x11_surface->surface);
//	assert(erased == 1); // TODO: assert fails
	erased = server->views_by_id.erase(view->id);
	assert(erased == 1);
}

void xwayland_surface_request_move(wl_listener* listener, void* data) {
	ZenithX11View* view = wl_container_of(listener, view, request_move);

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->flutter_engine_state->messenger.Send("request_move", result->data(),
	                                                   result->size());
}

void xwayland_surface_request_resize(wl_listener* listener, void* data) {
	ZenithX11View* view = wl_container_of(listener, view, request_resize);
	auto* event = static_cast<wlr_xwayland_resize_event*>(data);

	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("view_id"), EncodableValue((int64_t) view->id)},
		  {EncodableValue("edges"),   EncodableValue(event->edges)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->flutter_engine_state->messenger.Send("request_resize", result->data(),
	                                                   result->size());
}

void xwayland_surface_set_geometry(wl_listener* listener, void* data) {
	ZenithX11View* view = wl_container_of(listener, view, set_geometry);
	wlr_xwayland_surface* x11_surface = view->x11_surface;
	wlr_surface* surface = x11_surface->surface;
	ZenithServer* server = view->server;

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

	{
		std::scoped_lock lock(surface_framebuffer->mutex);
		surface_framebuffer->schedule_resize(surface->current.width, surface->current.height);
	}

	auto map = EncodableMap{
		  {EncodableValue("view_id"),              EncodableValue((int64_t) view->id)},
		  {EncodableValue("surface_role"),         EncodableValue(WLR_XDG_SURFACE_ROLE_TOPLEVEL)},
		  {EncodableValue("geometry_changed"),     EncodableValue(true)},
		  {EncodableValue("visible_bounds"),       EncodableValue(EncodableMap{
				{EncodableValue("x"),      EncodableValue(0)},
				{EncodableValue("y"),      EncodableValue(0)},
				{EncodableValue("width"),  EncodableValue(x11_surface->width)},
				{EncodableValue("height"), EncodableValue(x11_surface->height)},
		  })},
		  {EncodableValue("surface_size_changed"), EncodableValue(true)},
		  {EncodableValue("surface_width"),        EncodableValue(surface->current.width)},
		  {EncodableValue("surface_height"),       EncodableValue(surface->current.height)},
	};

	auto value = EncodableValue(map);

	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->flutter_engine_state->messenger.Send("configure_surface", result->data(),
	                                                   result->size());
}
