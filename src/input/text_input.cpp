#include <cassert>
#include "server.hpp"
#include "text_input.hpp"
#include "surfaces/zenith_surface.hpp"

using namespace flutter;

ZenithTextInput::ZenithTextInput(wlr_text_input_v3* wlr_text_input)
	  : wlr_text_input(wlr_text_input) {

	text_input_enable.notify = text_input_enable_handle;
	wl_signal_add(&wlr_text_input->events.enable, &text_input_enable);

	text_input_disable.notify = text_input_disable_handle;
	wl_signal_add(&wlr_text_input->events.disable, &text_input_disable);

	text_input_commit.notify = text_input_commit_handle;
	wl_signal_add(&wlr_text_input->events.commit, &text_input_commit);

	text_input_destroy.notify = text_input_destroy_handle;
	wl_signal_add(&wlr_text_input->events.destroy, &text_input_destroy);

	auto* server = ZenithServer::instance();
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
	if (wlr_text_input->focused_surface == nullptr) {
		return;
	}
	if (wlr_text_input->current_enabled) {
		disable();
	}
	assert(wlr_text_input->focused_surface != nullptr);
	wlr_text_input_v3_send_leave(wlr_text_input);
}

void ZenithTextInput::disable() const {
	assert(wlr_text_input->focused_surface != nullptr);
	auto* view = static_cast<ZenithSurface*>(wlr_text_input->focused_surface->data);
	view->active_text_input = nullptr;
	ZenithServer::instance()->embedder_state->send_text_input_event(view->id, TextInputEventType::disable);
}

void text_input_create_handle(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_text_input);
	auto* wlr_text_input = static_cast<wlr_text_input_v3*>(data);
	auto text_input = new ZenithTextInput(wlr_text_input);
	server->text_inputs.insert(text_input);
}

void text_input_enable_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_enable);
	wlr_text_input_v3* wlr_text_input = text_input->wlr_text_input;

	if (wlr_text_input->focused_surface == nullptr) {
		return;
	}

	wlr_surface* surface = wlr_text_input->focused_surface;

	auto* view = static_cast<ZenithSurface*>(surface->data);
	view->active_text_input = text_input;

	ZenithServer::instance()->embedder_state->send_text_input_event(view->id, TextInputEventType::enable);
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

	wlr_surface* surface = wlr_text_input->focused_surface;

	auto* view = static_cast<ZenithSurface*>(surface->data);
	ZenithServer::instance()->embedder_state->send_text_input_event(view->id, TextInputEventType::commit);
}

void text_input_destroy_handle(wl_listener* listener, void* data) {
	ZenithTextInput* text_input = wl_container_of(listener, text_input, text_input_destroy);
	wlr_text_input_v3* wlr_text_input = text_input->wlr_text_input;
	auto* server = ZenithServer::instance();

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
