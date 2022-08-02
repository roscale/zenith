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

void ZenithTextInput::enter(wlr_surface* surface) const {
	wlr_text_input_v3_send_enter(wlr_text_input, surface);
}

void ZenithTextInput::leave() const {
	if (wlr_text_input->focused_surface != nullptr && wlr_text_input->current_enabled) {
		disable();
	}
	wlr_text_input_v3_send_leave(wlr_text_input);
}

void ZenithTextInput::disable() const {
	auto it = server->view_id_by_wlr_surface.find(wlr_text_input->focused_surface);
	assert(it != server->view_id_by_wlr_surface.end());
	size_t id = it->second;
	std::unique_ptr<ZenithView>& view = server->views_by_id.find(id)->second;
	view->active_text_input = nullptr;

	send_text_input_disabled(view->server->embedder_state->messenger, view->id);
}

void text_input_enable_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_enable);
	auto it = text_input->server->view_id_by_wlr_surface.find(text_input->wlr_text_input->focused_surface);
	size_t id = it->second;
	std::unique_ptr<ZenithView>& view = text_input->server->views_by_id.find(id)->second;
	view->active_text_input = text_input;

	send_text_input_enabled(view->server->embedder_state->messenger, view->id);
}

void text_input_disable_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_disable);

	if (text_input->wlr_text_input->focused_surface != nullptr) {
		text_input->disable();
	}
}

void text_input_commit_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_commit);

	auto it = text_input->server->view_id_by_wlr_surface.find(text_input->wlr_text_input->focused_surface);
	assert(it != text_input->server->view_id_by_wlr_surface.end());
	size_t id = it->second;
	std::unique_ptr<ZenithView>& view = text_input->server->views_by_id.find(id)->second;

	send_text_input_committed(view->server->embedder_state->messenger, view->id);
}

void text_input_destroy_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_destroy);
	ZenithServer* server = text_input->server;
	server->text_inputs.remove_if(
		  [text_input](const std::unique_ptr<ZenithTextInput>& ptr) {
			  return ptr.get() == text_input;
		  });
}
