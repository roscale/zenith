
#include <assert.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/render/dmabuf.h>
#include <wlr/render/interface.h>
#include <wlr/backend/drm.h>
#include <wlr/render/gles2.h>
#include <wlr/render/egl.h>
#include <stdlib.h>

#include "embedder.h"

struct tinywl_server {
	struct wl_display* wl_display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;

	struct wlr_output_layout* output_layout;
	struct wl_list outputs;
	struct wl_listener new_output;
};

struct tinywl_output {
	struct wl_list link;
	struct tinywl_server* server;
	struct wlr_output* wlr_output;
	struct wl_listener frame;
};

FlutterEngine RunFlutter(struct tinywl_server* server);

static void server_new_output(struct wl_listener* listener, void* data) {
	struct tinywl_server* server =
		  wl_container_of(listener, server, new_output);
	struct wlr_output* wlr_output = data;

	if (!wl_list_empty(&wlr_output->modes)) {
		struct wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
		wlr_output_set_mode(wlr_output, mode);
		wlr_output_enable(wlr_output, true);
	}

	/* Allocates and configures our state for this output */
	struct tinywl_output* output = calloc(1, sizeof(struct tinywl_output));
	output->wlr_output = wlr_output;
	output->server = server;

//	output->frame.notify = output_frame;
//	wl_signal_add(&wlr_output->events.frame, &output->frame);
	wl_list_insert(&server->outputs, &output->link);

	wlr_output_layout_add_auto(server->output_layout, wlr_output);

//	if (!wlr_output_attach_render(output->wlr_output, NULL)) {
//		printf("NOOOO\n");
//		return;
//	}

	/* The "effective" resolution can change if you rotate your outputs. */
	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	/* Begin the renderer (calls glViewport and some other GL sanity checks) */
//	wlr_renderer_begin(server->renderer, width, height);

//	float color[4] = {0.6, 0.3, 0.3, 1.0};
//	wlr_renderer_clear(server->renderer, color);

//	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
//	glClear(GL_COLOR_BUFFER_BIT);

//	wlr_renderer_end(server->renderer);
//	wlr_output_commit(output->wlr_output);

//	printf("HAHA ");

	FlutterWindowMetricsEvent event = {};
	event.struct_size = sizeof(event);
	event.width = width;
	event.height = height;
	event.pixel_ratio = 1.0;
//	printf("%zu %zu ", event.width, event.height);

	FlutterEngine engine = RunFlutter(server);

	FlutterEngineSendWindowMetricsEvent(engine, &event);
}

//static const size_t kInitialWindowWidth = 800;
//static const size_t kInitialWindowHeight = 600;

//void GLFWcursorPositionCallbackAtPhase(GLFWwindow* window,
//                                       FlutterPointerPhase phase, double x,
//                                       double y) {
//	FlutterPointerEvent event = {};
//	event.struct_size = sizeof(event);
//	event.phase = phase;
//	event.x = x;
//	event.y = y;
//	event.timestamp =
//		  std::chrono::duration_cast<std::chrono::microseconds>(
//				std::chrono::high_resolution_clock::now().time_since_epoch())
//				.count();
//	FlutterEngineSendPointerEvent(
//		  reinterpret_cast<FlutterEngine>(glfwGetWindowUserPointer(window)), &event,
//		  1);
//}

//void GLFWcursorPositionCallback(GLFWwindow* window, double x, double y) {
//	GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kMove, x, y);
//}

//void GLFWmouseButtonCallback(GLFWwindow* window, int key, int action,
//                             int mods) {
//	if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
//		double x, y;
//		glfwGetCursorPos(window, &x, &y);
//		GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kDown, x, y);
//		glfwSetCursorPosCallback(window, GLFWcursorPositionCallback);
//	}
//
//	if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
//		double x, y;
//		glfwGetCursorPos(window, &x, &y);
//		GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kUp, x, y);
//		glfwSetCursorPosCallback(window, nullptr);
//	}
//}

//static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode,
//                            int action, int mods) {
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
//		glfwSetWindowShouldClose(window, GLFW_TRUE);
//	}
//}

//void GLFWwindowSizeCallback(GLFWwindow* window, int width, int height) {
//	FlutterWindowMetricsEvent event = {};
//	event.struct_size = sizeof(event);
//	event.width = width;
//	event.height = height;
//
//	event.pixel_ratio = 1.0;
//	FlutterEngineSendWindowMetricsEvent(
//		  reinterpret_cast<FlutterEngine>(glfwGetWindowUserPointer(window)),
//		  &event);
//}

bool made_current = true;

bool flutter_make_current(void* userdata) {
//	printf("MAKE_CURRENT\n");
//	fflush(stdout);

	struct tinywl_server* server = userdata;
	struct tinywl_output* output;

	wl_list_for_each(output, &server->outputs, link) {
		if (output->wlr_output->back_buffer == NULL) {
//			printf("\nF\n");
			if (!wlr_output_attach_render(output->wlr_output, NULL)) {
				return false;
			}
			int width, height;
			wlr_output_effective_resolution(output->wlr_output, &width, &height);
			/* Begin the renderer (calls glViewport and some other GL sanity checks) */
			wlr_renderer_begin(server->renderer, width, height);
		}
	}

	return wlr_egl_make_current(wlr_gles2_renderer_get_egl(server->renderer));

//		struct tinywl_server* server = userdata;
//		struct tinywl_output* output;
//		wl_list_for_each(output, &server->outputs, link) {
//			wlr_output_rollback(output->wlr_output);
//		}
//		made_current = true;

//		wl_list_for_each(output, &server->outputs, link) {
//			if (!wlr_output_attach_render(output->wlr_output, NULL)) {
//				return false;
//			}
//		}
//		int width, height;

//		wlr_output_effective_resolution(output->wlr_output, &width, &height);
	/* Begin the renderer (calls glViewport and some other GL sanity checks) */
//		wlr_renderer_begin(server->renderer, width, height);

//		float color[4] = {0.5, 0.3, 0.3, 1.0};
//		wlr_renderer_clear(server->renderer, color);
//	return true;
}

bool flutter_clear_current(void* userdata) {
//	printf("CLEAR_CURRENT\n");
//	fflush(stdout);
	struct tinywl_server* server = userdata;
	struct tinywl_output* output;
//	wl_list_for_each(output, &server->outputs, link) {
//		wlr_output_rollback(output->wlr_output);
//	}
//	made_current = false;

	return wlr_egl_unset_current(wlr_gles2_renderer_get_egl(server->renderer));

//	return true;
}

bool flutter_present(void* userdata) {
//	printf("PRESENT\n");
//	fflush(stdout);
	struct tinywl_server* server = userdata;
	struct tinywl_output* output;
	wl_list_for_each(output, &server->outputs, link) {
		struct wlr_renderer* renderer = output->server->renderer;
		wlr_renderer_end(renderer);
		if (!output->wlr_output->frame_pending) {
			wlr_output_commit(output->wlr_output);
		}

		if (output->wlr_output->back_buffer == NULL) {
//			printf("\nF\n");
			if (!wlr_output_attach_render(output->wlr_output, NULL)) {
				return false;
			}
		}
		int width, height;
		wlr_output_effective_resolution(output->wlr_output, &width, &height);
		/* Begin the renderer (calls glViewport and some other GL sanity checks) */
		wlr_renderer_begin(server->renderer, width, height);
	}
	made_current = false;
	return true;
}

uint32_t flutter_fbo_callback(void* userdata) {
	struct tinywl_server* server = userdata;
	uint32_t fbo = wlr_gles2_renderer_get_current_fbo(server->renderer);
//	printf("FBO %d\n", fbo);
	return fbo;
}

FlutterTransformation flutter_surface_transformation(void* userdata) {
	struct tinywl_server* server = userdata;
	struct tinywl_output* output;
	FlutterTransformation transformation;

	wl_list_for_each(output, &server->outputs, link) {
		int width, height;
		wlr_output_effective_resolution(output->wlr_output, &width, &height);
		transformation.scaleX = 1;
		transformation.scaleY = -1;
		transformation.pers2 = 1;
		transformation.transY = height;
	}
	return transformation;
}

FlutterEngine RunFlutter(struct tinywl_server* server) {
	FlutterRendererConfig config = {};
	config.type = kOpenGL;
	config.open_gl.struct_size = sizeof(config.open_gl);
	config.open_gl.make_current = flutter_make_current;
	config.open_gl.clear_current = flutter_clear_current;
	config.open_gl.present = flutter_present;
	config.open_gl.fbo_callback = flutter_fbo_callback;
	config.open_gl.surface_transformation = flutter_surface_transformation;

#define MY_PROJECT "/home/roscale/CLionProjects/flutter_embedder/data"
	FlutterProjectArgs args = {
		  .struct_size = sizeof(FlutterProjectArgs),
		  .assets_path = MY_PROJECT "/flutter_assets", // This directory is generated by `flutter build bundle`
		  .icu_data_path = MY_PROJECT "/icudtl.dat", // Find this in your bin/cache directory.
	};
	FlutterEngine engine = NULL;
	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, // renderer
	                              &args, server, &engine);

	assert(result == kSuccess && engine != NULL);

	//	glfwSetWindowUserPointer(window, engine);

//	GLFWwindowSizeCallback(window, kInitialWindowWidth, kInitialWindowHeight);

	return engine;
}

#include <signal.h>

int main(int argc, const char* argv[]) {
	wlr_log_init(WLR_DEBUG, NULL);

	struct tinywl_server server;
	server.wl_display = wl_display_create();
	server.backend = wlr_backend_autocreate(server.wl_display);
	server.renderer = wlr_backend_get_renderer(server.backend);
	wlr_renderer_init_wl_display(server.renderer, server.wl_display);

	wlr_compositor_create(server.wl_display, server.renderer);
	wlr_data_device_manager_create(server.wl_display);

	server.output_layout = wlr_output_layout_create();

	wl_list_init(&server.outputs);
	server.new_output.notify = server_new_output;
	wl_signal_add(&server.backend->events.new_output, &server.new_output);

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

	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
	        socket);
	wl_display_run(server.wl_display);

	wl_display_destroy_clients(server.wl_display);
	wl_display_destroy(server.wl_display);

//	glfwSetKeyCallback(window, GLFWKeyCallback);
//
//	glfwSetWindowSizeCallback(window, GLFWwindowSizeCallback);
//
//	glfwSetMouseButtonCallback(window, GLFWmouseButtonCallback);
//
//	while (!glfwWindowShouldClose(window)) {
//    std::cout << "Looping..." << std::endl;
//		glfwWaitEvents();
//	}

//	glfwDestroyWindow(window);
//	glfwTerminate();

	return EXIT_SUCCESS;
}