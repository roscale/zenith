#pragma once

#include <wayland-server.h>
#include <list>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <thread>
#include "output.hpp"
#include "keyboard.hpp"
#include "pointer/pointer.hpp"
#include "view/view.hpp"

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
	wlr_compositor* compositor;
	wlr_xdg_shell* xdg_shell;
	wlr_xwayland* xwayland;

	wlr_output_layout* output_layout;
	std::unique_ptr<ZenithOutput> output;

	wl_listener new_output{};
	wl_listener new_xdg_surface{};
	wl_listener new_xwayland_surface{};

	std::unordered_map<wlr_surface*, size_t> view_id_by_wlr_surface{};
	std::unordered_map<size_t, std::unique_ptr<ZenithView>> views_by_id{};
	std::unordered_map<size_t, std::shared_ptr<SurfaceFramebuffer>> surface_framebuffers{};
	std::mutex surface_framebuffers_mutex{};

	wlr_seat* seat;
	std::unique_ptr<ZenithPointer> pointer;
	std::list<std::unique_ptr<ZenithKeyboard>> keyboards{};

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

void server_new_xwayland_surface(wl_listener* listener, void* data);

void surface_commit(wl_listener* listener, void* data);

/*
 * This event is raised by the backend when a new input device becomes available.
 */
void server_new_input(wl_listener* listener, void* data);

/*
 * This event is raised by the seat when a client provides a cursor image.
 */
void server_seat_request_cursor(wl_listener* listener, void* data);
