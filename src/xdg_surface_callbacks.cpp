#include <src/platform_channels/method_channel.h>
#include "xdg_surface_callbacks.hpp"
#include "zenith_structs.hpp"
#include "input_callbacks.hpp"

extern "C" {
#include <malloc.h>
#define static
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/wlr_texture.h>
#undef static
}

void server_new_xdg_surface(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_xdg_surface);
	auto* xdg_surface = static_cast<wlr_xdg_surface*>(data);

	/* Allocate a ZenithView for this surface */
	auto* view = static_cast<ZenithView*>(calloc(1, sizeof(ZenithView)));
	view->server = server;
	view->xdg_surface = xdg_surface;

	/* Listen to the various events it can emit */
	view->map.notify = xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);

	view->unmap.notify = xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);

	view->destroy.notify = xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	/* Add it to the list of views. */
	server->views.insert(view);
}

void xdg_surface_map(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, map);
	view->mapped = true;
	focus_view(view);

	wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);
	FlutterEngineRegisterExternalTexture(view->server->output->engine, (int64_t) texture);

	using namespace flutter;

	switch (view->xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			auto value = EncodableValue(EncodableMap{
					{EncodableValue("texture_id"),  EncodableValue((int64_t) texture)},
					{EncodableValue("view_ptr"),    EncodableValue((int64_t) view)},
					{EncodableValue("surface_ptr"), EncodableValue((int64_t) view->xdg_surface->surface)},
					{EncodableValue("width"),       EncodableValue(texture->width)},
					{EncodableValue("height"),      EncodableValue(texture->height)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->messenger.Send("window_mapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP: {
			wlr_xdg_popup* popup = view->xdg_surface->popup;
			auto value = EncodableValue(EncodableMap{
					{EncodableValue("texture_id"),         EncodableValue((int64_t) texture)},
					{EncodableValue("view_ptr"),           EncodableValue((int64_t) view)},
					{EncodableValue("surface_ptr"),        EncodableValue((int64_t) view->xdg_surface->surface)},
					{EncodableValue("parent_surface_ptr"), EncodableValue((int64_t) popup->parent)},
					{EncodableValue("x"),                  EncodableValue(popup->geometry.x)},
					{EncodableValue("y"),                  EncodableValue(popup->geometry.y)},
					{EncodableValue("width"),              EncodableValue(popup->geometry.width)},
					{EncodableValue("height"),             EncodableValue(popup->geometry.height)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->messenger.Send("popup_mapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
}

void xdg_surface_unmap(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, unmap);
	view->mapped = false;

	wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);

	using namespace flutter;

	switch (view->xdg_surface->role) {
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
			auto value = EncodableValue(EncodableMap{
					{EncodableValue("texture_id"), EncodableValue((int64_t) texture)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->messenger.Send("window_unmapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_POPUP: {
			wlr_xdg_popup* popup = view->xdg_surface->popup;

			auto value = EncodableValue(EncodableMap{
					{EncodableValue("view_ptr"),           EncodableValue((int64_t) view)},
					{EncodableValue("parent_surface_ptr"), EncodableValue((int64_t) popup->parent)},
			});
			auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);

			view->server->output->messenger.Send("popup_unmapped", result->data(), result->size());
			break;
		}
		case WLR_XDG_SURFACE_ROLE_NONE:
			break;
	}
}

void xdg_surface_destroy(wl_listener* listener, void* data) {
	ZenithView* view = wl_container_of(listener, view, destroy);
	wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);

	FlutterEngineUnregisterExternalTexture(view->server->output->engine, (int64_t) texture);

	view->server->views.erase(view);
	free(view);
}