#include <cassert>
#include "server.hpp"
#include "text_input.hpp"
#include "messages.hpp"

using namespace flutter;

ZenithTextInput::ZenithTextInput(ZenithServer* server, wlr_text_input_v3* wlr_text_input)
	  : server(server), wlr_text_input(wlr_text_input) {

	text_input_enable.notify = text_input_enable_handle;
	wl_signal_add(&wlr_text_input->events.enable, &text_input_enable);

	text_input_disable.notify = text_input_disable_handle;
	wl_signal_add(&wlr_text_input->events.disable, &text_input_disable);

	text_input_commit.notify = text_input_commit_handle;
	wl_signal_add(&wlr_text_input->events.commit, &text_input_commit);

	text_input_destroy.notify = text_input_destroy_handle;
	wl_signal_add(&wlr_text_input->events.destroy, &text_input_destroy);

	wlr_surface* focused_surface = server->seat->keyboard_state.focused_surface;
	if (focused_surface != nullptr &&
	    wl_resource_get_client(focused_surface->resource) == wl_resource_get_client(wlr_text_input->resource)) {
		wlr_text_input_v3_send_enter(wlr_text_input, server->seat->keyboard_state.focused_surface);
	}
}

void ZenithTextInput::enter(wlr_xdg_surface* xdg_surface) const {
	// We take an xdg_surface and not a wlr_surface as argument because I want to enforce at compile-time having
	// a wlr_surface associated with an xdg_surface.
	wlr_text_input_v3_send_enter(wlr_text_input, xdg_surface->surface);
}

void ZenithTextInput::leave() const {
	if (wlr_text_input->current_enabled and wlr_text_input->focused_surface != nullptr) {
		disable();
	}
	wlr_text_input_v3_send_leave(wlr_text_input);
}

void ZenithTextInput::disable() const {
	assert(wlr_text_input->focused_surface != nullptr);

	wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(wlr_text_input->focused_surface);

	auto* view = static_cast<ZenithView*>(xdg_surface->data);
	view->active_text_input = nullptr;

	send_text_input_disabled(view->server->embedder_state->messenger, view->id);
}

void text_input_enable_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_enable);
	wlr_text_input_v3* wlr_text_input = text_input->wlr_text_input;

	if (wlr_text_input->focused_surface == nullptr) {
		return;
	}

	wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(wlr_text_input->focused_surface);

	auto* view = static_cast<ZenithView*>(xdg_surface->data);
	view->active_text_input = text_input;

	send_text_input_enabled(view->server->embedder_state->messenger, view->id);
}

void text_input_disable_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_disable);
	wlr_text_input_v3* wlr_text_input = text_input->wlr_text_input;

	if (wlr_text_input->focused_surface != nullptr) {
		text_input->disable();
	}
}

void text_input_commit_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_commit);
	wlr_text_input_v3* wlr_text_input = text_input->wlr_text_input;

	if (!wlr_text_input->current_enabled || wlr_text_input->focused_surface == nullptr) {
		return;
	}

	wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(wlr_text_input->focused_surface);

	auto* view = static_cast<ZenithView*>(xdg_surface->data);
	send_text_input_committed(view->server->embedder_state->messenger, view->id);
}

void text_input_destroy_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_destroy);
	wlr_text_input_v3* wlr_text_input = text_input->wlr_text_input;
	ZenithServer* server = text_input->server;

	if (wlr_text_input->current_enabled && wlr_text_input->focused_surface != nullptr) {
		text_input->disable();
	}

	bool erased = server->text_inputs.erase(text_input);
	assert(erased == 1);

	wl_list_remove(&text_input->text_input_enable.link);
	wl_list_remove(&text_input->text_input_disable.link);
	wl_list_remove(&text_input->text_input_commit.link);
	wl_list_remove(&text_input->text_input_destroy.link);

	delete text_input;
}
