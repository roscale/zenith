#include "xdg_surface_callbacks.hpp"
#include "flutland_structs.hpp"

extern "C" {
#define static
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/wlr_texture.h>
#undef static

#include <malloc.h>
}

void server_new_xdg_surface(struct wl_listener* listener, void* data) {
	/* This event is raised when wlr_xdg_shell receives a new xdg surface from a
	 * client, either a toplevel (application window) or popup. */
	struct flutland_server* server =
		  wl_container_of(listener, server, new_xdg_surface);
	auto* xdg_surface = static_cast<wlr_xdg_surface*>(data);
	if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		return;
	}

	/* Allocate a flutland_view for this surface */
	auto* view = static_cast<flutland_view*>(calloc(1, sizeof(struct flutland_view)));
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
	wl_list_insert(&server->views, &view->link);

	printf("\nNEW CLIENT\n\n");
}

void xdg_surface_map(struct wl_listener* listener, void* data) {
	printf("MAP SURFACE\n");
	fflush(stdout);
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct flutland_view* view = wl_container_of(listener, view, map);
	view->mapped = true;

	struct wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);
	FlutterEngineRegisterExternalTexture(view->server->output->engine, (int64_t) texture);

	using namespace flutter;
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("texture_id"), EncodableValue((int64_t) texture)},
		  {EncodableValue("width"),      EncodableValue(texture->width)},
		  {EncodableValue("height"),     EncodableValue(texture->height)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->output->messenger.Send("window_mapped", result->data(), result->size());
}

void xdg_surface_unmap(struct wl_listener* listener, void* data) {
	printf("UNMAP SURFACE\n");
	fflush(stdout);
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct flutland_view* view = wl_container_of(listener, view, unmap);
	view->mapped = false;

	struct wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);

//	auto value = flutter::EncodableValue((int64_t) texture);
//	auto result = flutter::StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
//	view->server->output->messenger.Send("window_unmapped", result->data(), result->size());

	using namespace flutter;
	auto value = EncodableValue(EncodableMap{
		  {EncodableValue("texture_id"), EncodableValue((int64_t) texture)},
	});
	auto result = StandardMethodCodec::GetInstance().EncodeSuccessEnvelope(&value);
	view->server->output->messenger.Send("window_unmapped", result->data(), result->size());

	FlutterEngineUnregisterExternalTexture(view->server->output->engine, (int64_t) texture);
}

void xdg_surface_destroy(struct wl_listener* listener, void* data) {
	printf("DESTROY SURFACE\n");
	fflush(stdout);
	/* Called when the surface is destroyed and should never be shown again. */
	struct flutland_view* view = wl_container_of(listener, view, destroy);
	wl_list_remove(&view->link);
	free(view);
}