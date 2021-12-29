#include "server.hpp"
#include <unistd.h>

extern "C" {
#define static
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/util/log.h>
#include <wlr/render/gles2.h>
#undef static
}

ZenithServer* ZenithServer::_instance = nullptr;

ZenithServer* ZenithServer::instance() {
	if (_instance == nullptr) {
		_instance = new ZenithServer();
	}
	return _instance;
}

ZenithServer::ZenithServer() {
	display = wl_display_create();
	backend = wlr_backend_autocreate(display);
	renderer = wlr_backend_get_renderer(backend);
	wlr_renderer_init_wl_display(renderer, display);

	wlr_compositor_create(display, renderer);
	wlr_data_device_manager_create(display);

	output_layout = wlr_output_layout_create();

	new_output.notify = server_new_output;
	wl_signal_add(&backend->events.new_output, &new_output);

	xdg_shell = wlr_xdg_shell_create(display);
	new_xdg_surface.notify = server_new_xdg_surface;
	wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);

	/*
	 * Configures a seat, which is a single "seat" at which a user sits and
	 * operates the computer. This conceptually includes up to one keyboard,
	 * pointer, touch, and drawing tablet device. We also rig up a listener to
	 * let us know when new input devices are available on the backend.
	 */
	new_input.notify = server_new_input;
	wl_signal_add(&backend->events.new_input, &new_input);
	seat = wlr_seat_create(display, "seat0");

	request_cursor.notify = server_seat_request_cursor;
	wl_signal_add(&seat->events.request_set_cursor,
	              &request_cursor);
}

void ZenithServer::run() {
	const char* socket = wl_display_add_socket_auto(display);
	if (!socket) {
		wlr_backend_destroy(backend);
		exit(1);
	}

	if (!wlr_backend_start(backend)) {
		wlr_backend_destroy(backend);
		wl_display_destroy(display);
		exit(1);
	}

	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);

	// Set the WAYLAND_DISPLAY environment variable to our socket and start a few clients to test things.
	setenv("WAYLAND_DISPLAY", socket, true);
	setenv("XDG_SESSION_TYPE", "wayland", true);
//	setenv("KDE_FULL_SESSION", "1", true);

	if (fork() == 0) {
		execl("/bin/sh", "/bin/sh", "-c", "konsole", nullptr);
	}
	wl_display_run(display);

	wl_display_destroy_clients(display);
	wl_display_destroy(display);
}

void server_new_output(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_output);
	auto* wlr_output = static_cast<struct wlr_output*>(data);

	if (server->output != nullptr) {
		// Allow only one output for the time being.
		return;
	}

	if (!wl_list_empty(&wlr_output->modes)) {
		// Set the preferred resolution and refresh rate of the monitor which will probably be the highest one.
		wlr_output_enable(wlr_output, true);
		wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
		wlr_output_set_mode(wlr_output, mode);

		if (!wlr_output_commit(wlr_output)) {
			return;
		}
	}

	// Create the output.
	auto output = std::make_unique<ZenithOutput>(server, wlr_output);
	wlr_output_layout_add_auto(server->output_layout, wlr_output);

	// Create the flutter engine associated with this output.
	wlr_egl* main_egl = wlr_gles2_renderer_get_egl(server->renderer);
	auto flutter_engine_state = std::make_unique<FlutterEngineState>(output.get(), main_egl);

	// Link them together
	output->flutter_engine_state = std::move(flutter_engine_state);

	// Run the engine.
	output->flutter_engine_state->run_engine();

	server->output = std::move(output);
}

void server_new_xdg_surface(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_xdg_surface);
	auto* xdg_surface = static_cast<wlr_xdg_surface*>(data);

	/* Allocate a ZenithView for this surface */
	auto view = std::make_unique<ZenithView>(server, xdg_surface);

	/* Add it to the list of views. */
	server->view_id_by_wlr_surface.insert_or_assign(view->xdg_surface->surface, view->id);
	server->views_by_id.insert(std::make_pair(view->id, std::move(view)));
}

void server_new_input(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_input);
	auto* device = static_cast<wlr_input_device*>(data);

	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD: {
			auto keyboard = std::make_unique<ZenithKeyboard>(server, device);
			server->keyboards.push_back(std::move(keyboard));
			break;
		}
		case WLR_INPUT_DEVICE_POINTER: {
			if (server->pointer == nullptr) {
				server->pointer = std::make_unique<ZenithPointer>(server);
			}
			/*
			 * We don't do anything special with pointers. All of our pointer handling
			 * is proxied through wlr_cursor. On another compositor, you might take this
			 * opportunity to do libinput configuration on the device to set
			 * acceleration, etc.
			 */
			wlr_cursor_attach_input_device(server->pointer->cursor, device);
			break;
		}
		default:
			break;
	}
	/* We need to let the wlr_seat know what our capabilities are, which is
	 * communicated to the client. In TinyWL we always have a cursor, even if
	 * there are no pointer devices, so we always include that capability. */
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!server->keyboards.empty()) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat, caps);
}

void server_seat_request_cursor(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, request_cursor);

	auto* event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);
	wlr_seat_client* focused_client = server->seat->pointer_state.focused_client;
	/* This can be sent by any client, so we check to make sure this one is
	 * actually has pointer focus first. */
	if (focused_client == event->seat_client) {
		/* Once we've vetted the client, we can tell the cursor to use the
		 * provided surface as the cursor image. It will set the hardware cursor
		 * on the output that it's currently on and continue to do so as the
		 * cursor moves between outputs. */
		wlr_cursor_set_surface(server->pointer->cursor, event->surface, event->hotspot_x, event->hotspot_y);
	}
}
