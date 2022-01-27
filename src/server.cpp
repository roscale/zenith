#include "server.hpp"
#include "debug.hpp"
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
	main_thread_id = std::this_thread::get_id();
	display = wl_display_create();
	if (display == nullptr) {
		wlr_log(WLR_ERROR, "Could not create Wayland display");
		exit(1);
	}

	backend = wlr_backend_autocreate(display);
	if (backend == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots backend");
		exit(2);
	}

	renderer = wlr_backend_get_renderer(backend);
	if (!wlr_renderer_init_wl_display(renderer, display)) {
		wlr_log(WLR_ERROR, "Could not initialize wlroots renderer");
		exit(3);
	}

	if (wlr_compositor_create(display, renderer) == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots compositor");
		exit(4);
	}

	if (wlr_data_device_manager_create(display) == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots data device manager");
		exit(5);
	}

	output_layout = wlr_output_layout_create();
	if (output_layout == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots output layout");
		exit(6);
	}

	xdg_shell = wlr_xdg_shell_create(display);
	if (xdg_shell == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots XDG shell");
		exit(7);
	}

	/*
	 * Configures a seat, which is a single "seat" at which a user sits and
	 * operates the computer. This conceptually includes up to one keyboard,
	 * pointer, touch, and drawing tablet device.
	 */
	seat = wlr_seat_create(display, "seat0");
	if (seat == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots seat");
		exit(8);
	}

	// Called at the start for each available output, but also when the user plugs in a monitor.
	new_output.notify = server_new_output;
	wl_signal_add(&backend->events.new_output, &new_output);

	new_xdg_surface.notify = server_new_xdg_surface;
	wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);

	// Called at the start for each available input device, but also when the user plugs in a new input
	// device, like a mouse, keyboard, drawing tablet, etc.
	new_input.notify = server_new_input;
	wl_signal_add(&backend->events.new_input, &new_input);

	// Programs can request to change the cursor image.
	request_cursor.notify = server_seat_request_cursor;
	wl_signal_add(&seat->events.request_set_cursor,
	              &request_cursor);
}

void ZenithServer::run(char* startup_command) {
	wlr_egl* main_egl = wlr_gles2_renderer_get_egl(renderer);
	flutter_engine_state = std::make_unique<FlutterEngineState>(this, main_egl);

	// Run the engine.
	flutter_engine_state->run_engine();

	const char* socket = wl_display_add_socket_auto(display);
	if (!socket) {
		wlr_log(WLR_ERROR, "Could not create a Wayland socket");
		wlr_backend_destroy(backend);
		exit(9);
	}

	if (!wlr_backend_start(backend)) {
		wlr_log(WLR_ERROR, "Could not start backend");
		wlr_backend_destroy(backend);
		wl_display_destroy(display);
		exit(10);
	}

	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);

	// Make sure the X11 session from the host is not visible because some programs prefer talking
	// to the X server instead of defaulting to Wayland.
	unsetenv("DISPLAY");

	setenv("WAYLAND_DISPLAY", socket, true);
	setenv("XDG_SESSION_TYPE", "wayland", true);

	if (fork() == 0) {
		execl("/bin/sh", "/bin/sh", "-c", startup_command, nullptr);
	}

	wl_display_run(display);

	wl_display_destroy_clients(display);
	wl_display_destroy(display);
}

size_t i = 1;

void server_new_output(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_output);
	auto* wlr_output = static_cast<struct wlr_output*>(data);

	static const char* selected_output_str = getenv("ZENITH_OUTPUT");
	static size_t selected_output = selected_output_str != nullptr
	                                ? selected_output_str[0] - '0'
	                                : 0;

	if (server->output != nullptr || i <= selected_output) {
		i += 1;
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

	// Tell Flutter how big the screen is, so it can start rendering.
	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = width;
	window_metrics.height = height;
	window_metrics.pixel_ratio = 1.0;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(server->renderer));
	server->flutter_engine_state->send_window_metrics(window_metrics);

	server->output = std::move(output);
}

void server_new_xdg_surface(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_xdg_surface);
	auto* xdg_surface = static_cast<wlr_xdg_surface*>(data);

	/* Allocate a ZenithView for this surface */
	auto view = std::make_unique<ZenithView>(server, xdg_surface);

	/* Add it to the list of views. */
	server->view_id_by_wlr_surface.insert(std::make_pair(view->xdg_surface->surface, view->id));
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
