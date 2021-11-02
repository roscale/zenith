#include "flutland_structs.hpp"
#include "output_callbacks.hpp"
#include "xdg_surface_callbacks.hpp"
#include "input_callbacks.hpp"

extern "C" {
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define static
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/render/dmabuf.h>
#include <wlr/render/interface.h>
#include <wlr/render/egl.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#undef static
}

int main(int argc, const char* argv[]) {
	wlr_log_init(WLR_DEBUG, nullptr);

	struct flutland_server server;
	server.output = nullptr;
	server.wl_display = wl_display_create();
	server.backend = wlr_backend_autocreate(server.wl_display);
	server.renderer = wlr_backend_get_renderer(server.backend);
	wlr_renderer_init_wl_display(server.renderer, server.wl_display);

	wlr_compositor_create(server.wl_display, server.renderer);
	wlr_data_device_manager_create(server.wl_display);

	server.output_layout = wlr_output_layout_create();

	server.new_output.notify = server_new_output;
	wl_signal_add(&server.backend->events.new_output, &server.new_output);

	wl_list_init(&server.views);
	server.xdg_shell = wlr_xdg_shell_create(server.wl_display);
	server.new_xdg_surface.notify = server_new_xdg_surface;
	wl_signal_add(&server.xdg_shell->events.new_surface, &server.new_xdg_surface);

	/*
	 * Creates a cursor, which is a wlroots utility for tracking the cursor
	 * image shown on screen.
	 */
	server.cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(server.cursor, server.output_layout);

	/* Creates an xcursor manager, another wlroots utility which loads up
	 * Xcursor themes to source cursor images from and makes sure that cursor
	 * images are available at all scale factors on the screen (necessary for
	 * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
	server.cursor_mgr = wlr_xcursor_manager_create(nullptr, 24);
	wlr_xcursor_manager_load(server.cursor_mgr, 1);

	/*
	 * wlr_cursor *only* displays an image on screen. It does not move around
	 * when the pointer moves. However, we can attach input devices to it, and
	 * it will generate aggregate events for all of them. In these events, we
	 * can choose how we want to process them, forwarding them to clients and
	 * moving the cursor around. More detail on this process is described in my
	 * input handling blog post:
	 *
	 * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
	 *
	 * And more comments are sprinkled throughout the notify functions above.
	 */
	server.cursor_motion.notify = server_cursor_motion;
	wl_signal_add(&server.cursor->events.motion, &server.cursor_motion);
	server.cursor_motion_absolute.notify = server_cursor_motion_absolute;
	wl_signal_add(&server.cursor->events.motion_absolute,
	              &server.cursor_motion_absolute);
	server.cursor_button.notify = server_cursor_button;
	wl_signal_add(&server.cursor->events.button, &server.cursor_button);
	server.cursor_axis.notify = server_cursor_axis;
	wl_signal_add(&server.cursor->events.axis, &server.cursor_axis);
	server.cursor_frame.notify = server_cursor_frame;
	wl_signal_add(&server.cursor->events.frame, &server.cursor_frame);

	/*
	 * Configures a seat, which is a single "seat" at which a user sits and
	 * operates the computer. This conceptually includes up to one keyboard,
	 * pointer, touch, and drawing tablet device. We also rig up a listener to
	 * let us know when new input devices are available on the backend.
	 */
	server.new_input.notify = server_new_input;
	wl_signal_add(&server.backend->events.new_input, &server.new_input);
	server.seat = wlr_seat_create(server.wl_display, "seat0");
//	server.request_cursor.notify = seat_request_cursor;
//	wl_signal_add(&server.seat->events.request_set_cursor,
//	              &server.request_cursor);
//	server.request_set_selection.notify = seat_request_set_selection;
//	wl_signal_add(&server.seat->events.request_set_selection,
//	              &server.request_set_selection);


	const char* socket = wl_display_add_socket_auto(server.wl_display);
	if (!socket) {
		wlr_backend_destroy(server.backend);
		return 1;
	}

	if (!wlr_backend_start(server.backend)) {
		wlr_backend_destroy(server.backend);
		wl_display_destroy(server.wl_display);
		return 1;
	}

	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);


	/* Set the WAYLAND_DISPLAY environment variable to our socket and run the startup command if requested. */
//	setenv("WAYLAND_DISPLAY", socket, true);
//	setenv("XDG_SESSION_TYPE", "wayland", true);
//	if (fork() == 0) {
//		execl("/bin/sh", "/bin/sh", "-c", "firefox www.ddg.gg", nullptr);
//	}

	struct wl_event_loop* event_loop = wl_display_get_event_loop(server.wl_display);
	int fd = wl_event_loop_get_fd(event_loop);

//#pragma clang diagnostic push
//#pragma ide diagnostic ignored "EndlessLoop"
//	while (true) {
//		struct pollfd pfd = {
//			  .fd = fd,
//			  .events = POLLIN,
//		};
//
//		poll(&pfd, 1, 0);
//		if (pfd.revents & POLLIN) {
//			wl_event_loop_dispatch(event_loop, 0);
//			wl_display_flush_clients(server.wl_display);
////			printf("\nb\n");
//		}
//	}
//#pragma clang diagnostic pop

	wl_display_run(server.wl_display);

	wl_display_destroy_clients(server.wl_display);
	wl_display_destroy(server.wl_display);

	return EXIT_SUCCESS;
}