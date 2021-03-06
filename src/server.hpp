#pragma once

#include <wayland-server.h>
#include <list>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <thread>
#include <unordered_set>
#include "output.hpp"
#include "input/keyboard.hpp"
#include "input/pointer.hpp"
#include "input/touch.hpp"
#include "view.hpp"
#include "point.hpp"

extern "C" {
#define static
#include <wlr/backend.h>
#define class class_variable
#include <wlr/xwayland.h>
#undef class
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#undef static
}

struct ZenithServer {
private:
	ZenithServer();

	static ZenithServer* _instance;

public:
	static ZenithServer* instance();

	void run(char* startup_command);

	std::thread::id main_thread_id;
	wl_display* display;
	wlr_backend* backend;
	wlr_renderer* renderer;
	wlr_allocator* allocator;
	wlr_compositor* compositor;
	wlr_xdg_shell* xdg_shell;

	wlr_output_layout* output_layout;
	std::unique_ptr<ZenithOutput> output; // We support a single output at the moment.

	// Box composed of all outputs.
	// A single Flutter engine doesn't support multi-window/multiscreen apps and cannot draw to multiple framebuffers.
	// If we are going to support multiple outputs in the future, we would have to create big framebuffer for all
	// outputs of the size of this box and reserve a portion of the framebuffer for each.
	// Right now, only one output is supported so the size of this box is essentially the size of the output.
	wlr_box output_layout_box;

	wl_listener new_output{};
	wl_listener new_xdg_surface{};

	std::unordered_map<wlr_surface*, size_t> view_id_by_wlr_surface{};
	std::unordered_map<size_t, std::unique_ptr<ZenithView>> views_by_id{};
	std::unordered_map<size_t, std::shared_ptr<Framebuffer>> surface_framebuffers{};
	std::mutex surface_framebuffers_mutex{};

	wlr_seat* seat;
	std::unique_ptr<ZenithPointer> pointer;
	std::list<std::unique_ptr<ZenithKeyboard>> keyboards{};
	std::list<std::unique_ptr<ZenithTouchDevice>> touch_devices{};
	std::unordered_map<int, Point> leaf_surface_coords_per_device_id{};

	wl_listener new_input{};
	wl_listener request_cursor{};

	std::unique_ptr<FlutterEngineState> flutter_engine_state{};
};

/*
 * This event is raised when a new output is detected, like a monitor or a projector.
 */
void server_new_output(wl_listener* listener, void* data);

/*
 * This event is raised when wlr_xdg_shell receives a new xdg surface from a
 * client, either a toplevel (application window) or popup.
 */
void server_new_xdg_surface(wl_listener* listener, void* data);

void surface_commit(wl_listener* listener, void* data);

/*
 * This event is raised by the backend when a new input device becomes available.
 */
void server_new_input(wl_listener* listener, void* data);

/*
 * This event is raised by the seat when a client provides a cursor image.
 */
void server_seat_request_cursor(wl_listener* listener, void* data);
