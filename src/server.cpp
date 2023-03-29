#include "server.hpp"
#include "debug.hpp"
#include "assert.hpp"
#include "util/egl/egl_extensions.hpp"
#include "egl/create_shared_egl_context.hpp"
#include "zenith_toplevel_decoration.hpp"
#include <unistd.h>
#include <sys/eventfd.h>

extern "C" {
#define static
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/render/allocator.h>
#include <wlr/backend/libinput.h>
#include <wlr/backend/drm.h>
#include <wlr/util/log.h>
#include <wlr/render/gles2.h>
#include <wlr/interfaces/wlr_touch.h>
#undef static
}

ZenithServer* ZenithServer::_instance = nullptr;

ZenithServer* ZenithServer::instance() {
	if (_instance == nullptr) {
		_instance = new ZenithServer();
	}
	return _instance;
}

static float read_display_scale();

ZenithServer::ZenithServer() {
	main_thread_id = std::this_thread::get_id();

	display_scale = read_display_scale();

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

	renderer = wlr_renderer_autocreate(backend);
	if (!wlr_renderer_init_wl_display(renderer, display)) {
		wlr_log(WLR_ERROR, "Could not initialize wlroots renderer");
		exit(3);
	}

	/*
	 * Auto-creates an allocator for us.
	 * The allocator is the bridge between the renderer and the backend. It handles the buffer creation,
	 * allowing wlroots to render onto the screen.
	 */
	allocator = wlr_allocator_autocreate(backend, renderer);
	if (allocator == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots allocator");
		exit(12);
	}

	compositor = wlr_compositor_create(display, renderer);
	if (compositor == nullptr) {
		wlr_log(WLR_ERROR, "Could not create wlroots compositor");
		exit(4);
	}
//	surface_destroyed.notify = server_surface_destroyed;
//	new_surface.notify = server_new_surface;
//	wl_signal_add(&compositor->events.new_surface, &new_surface);

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

	text_input_manager = wlr_text_input_manager_v3_create(display);
	if (text_input_manager == nullptr) {
		wlr_log(WLR_ERROR, "Could not create text input manager");
		exit(-1);
	}

	decoration_manager = wlr_xdg_decoration_manager_v1_create(display);
	if (decoration_manager == nullptr) {
		wlr_log(WLR_ERROR, "Could not create text input manager");
		exit(-1);
	}

	data_device_manager = wlr_data_device_manager_create(display);
	if (data_device_manager == nullptr) {
		wlr_log(WLR_ERROR, "Could not create text input manager");
		exit(-1);
	}

	// Called at the start for each available output, but also when the user plugs in a monitor.
	new_output.notify = output_create_handle;
	wl_signal_add(&backend->events.new_output, &new_output);

	new_surface.notify = zenith_surface_create;
	wl_signal_add(&compositor->events.new_surface, &new_surface);

	new_xdg_surface.notify = zenith_xdg_surface_create;
	wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);

	// Called at the start for each available input device, but also when the user plugs in a new input
	// device, like a mouse, keyboard, drawing tablet, etc.
	new_input.notify = server_new_input;
	wl_signal_add(&backend->events.new_input, &new_input);

	// Programs can request to change the cursor image.
	request_cursor.notify = server_seat_request_cursor;
	wl_signal_add(&seat->events.request_set_cursor, &request_cursor);

	new_text_input.notify = text_input_create_handle;
	wl_signal_add(&text_input_manager->events.text_input, &new_text_input);

	new_toplevel_decoration.notify = toplevel_decoration_create_handle;
	wl_signal_add(&decoration_manager->events.new_toplevel_decoration, &new_toplevel_decoration);

	request_set_selection.notify = server_seat_request_set_selection;
	wl_signal_add(&seat->events.request_set_selection, &request_set_selection);

	auto callable_queue_function = [](int fd, uint32_t mask, void* data) {
		auto* server = ZenithServer::instance();
		return (int) server->callable_queue.execute();
	};

	auto* event_loop = wl_display_get_event_loop(display);
	wl_event_loop_add_fd(event_loop, callable_queue.get_fd(), WL_EVENT_READABLE, callable_queue_function, nullptr);

	load_egl_extensions();
	// TODO: Implement drag and drop.
}

void ZenithServer::run(const char* startup_command) {
	this->startup_command = startup_command;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(renderer));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const char* socket = wl_display_add_socket_auto(display);
	if (!socket) {
		wlr_log(WLR_ERROR, "Could not create a Wayland socket");
		wlr_backend_destroy(backend);
		exit(9);
	}

	// Make sure the X11 session from the host is not visible because some programs prefer talking
	// to the X server instead of defaulting to Wayland.
	unsetenv("DISPLAY");

	setenv("WAYLAND_DISPLAY", socket, true);
	setenv("XDG_SESSION_TYPE", "wayland", true);
	setenv("GDK_BACKEND", "wayland", true); // Force GTK apps to run on Wayland.
	setenv("QT_QPA_PLATFORM", "wayland", true); // Force QT apps to run on Wayland.

	wlr_egl* main_egl = wlr_gles2_renderer_get_egl(renderer);

	// Create 2 OpenGL shared contexts for rendering operations.
	wlr_egl* flutter_gl_context = create_shared_egl_context(main_egl);
	wlr_egl* flutter_resource_gl_context = create_shared_egl_context(main_egl);

	embedder_state = std::make_unique<EmbedderState>(flutter_gl_context, flutter_resource_gl_context);

	// Run the engine.
	embedder_state->run_engine();

	if (!wlr_backend_start(backend)) {
		wlr_log(WLR_ERROR, "Could not start backend");
		wlr_backend_destroy(backend);
		wl_display_destroy(display);
		exit(10);
	}

	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);

	wl_display_run(display);

	wl_display_destroy_clients(display);
	wl_display_destroy(display);
}

static float read_display_scale() {
	static const char* display_scale_str = getenv("ZENITH_SCALE");
	if (display_scale_str == nullptr) {
		return 1.0f;
	}
	try {
		return std::stof(display_scale_str);
	} catch (std::invalid_argument&) {
		return 1.0f;
	} catch (std::out_of_range&) {
		return 1.0f;
	}
}

void server_new_input(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_input);
	auto* wlr_device = static_cast<wlr_input_device*>(data);

	switch (wlr_device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD: {
			auto keyboard = std::make_unique<ZenithKeyboard>(server, wlr_device);
			server->keyboards.push_back(std::move(keyboard));
			break;
		}
		case WLR_INPUT_DEVICE_POINTER: {
			if (server->pointer == nullptr) {
				// Regardless of the number of input devices, there will be only one pointer on the
				// screen if at least one input device exists.
				server->pointer = std::make_unique<ZenithPointer>(server);
			}

			bool is_touchpad = wlr_input_device_is_libinput(wlr_device);
			if (is_touchpad) {
				// Enable tapping by default on all touchpads.
				libinput_device* device = wlr_libinput_get_device_handle(wlr_device);
				libinput_device_config_tap_set_enabled(device, LIBINPUT_CONFIG_TAP_ENABLED);
				libinput_device_config_tap_set_drag_enabled(device, LIBINPUT_CONFIG_DRAG_ENABLED);
				libinput_device_config_scroll_set_natural_scroll_enabled(device, true);
				libinput_device_config_dwt_set_enabled(device, LIBINPUT_CONFIG_DWT_ENABLED);
			}

			wlr_cursor_attach_input_device(server->pointer->cursor, wlr_device);
			break;
		}
		case WLR_INPUT_DEVICE_TOUCH: {
			auto touch_device = std::make_unique<ZenithTouchDevice>(server, wlr_device);
			server->touch_devices.push_back(std::move(touch_device));
			break;
			// TODO: handle destruct callback
		}
		default:
			break;
	}

	uint32_t caps = 0;
	if (server->pointer != nullptr) {
		caps |= WL_SEAT_CAPABILITY_POINTER;
	}
	if (!server->keyboards.empty()) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	if (!server->touch_devices.empty()) {
		caps |= WL_SEAT_CAPABILITY_TOUCH;
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
		if (server->pointer != nullptr && server->pointer->is_visible()) {
			wlr_cursor_set_surface(server->pointer->cursor, event->surface, event->hotspot_x, event->hotspot_y);
		}
	}
}

void server_seat_request_set_selection(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, request_set_selection);
	auto* event = static_cast<wlr_seat_request_set_selection_event*>(data);
	wlr_seat_set_selection(server->seat, event->source, event->serial);
	// TODO: Add security. Don't let any client overwrite the clipboard randomly.
}

bool is_main_thread() {
	return std::this_thread::get_id() == ZenithServer::instance()->main_thread_id;
}
