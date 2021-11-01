#include <unistd.h>
#include "flutland_structs.hpp"
#include "output_callbacks.hpp"
#include "xdg_surface_callbacks.hpp"

extern "C" {
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
	while (true) {
		struct pollfd pfd = {
			  .fd = fd,
			  .events = POLLIN,
		};

		poll(&pfd, 1, 0);
		if (pfd.revents & POLLIN) {
			wl_event_loop_dispatch(event_loop, 0);
			wl_display_flush_clients(server.wl_display);
//			printf("\nb\n");
		}
	}
#pragma clang diagnostic pop

//	wl_display_run(server.wl_display);

	wl_display_destroy_clients(server.wl_display);
	wl_display_destroy(server.wl_display);

	return EXIT_SUCCESS;
}