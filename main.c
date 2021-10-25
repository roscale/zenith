
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
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/wlr_texture.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "embedder.h"

intptr_t g_baton;
pthread_mutex_t baton_mutex;

sem_t vsync_semaphore;

struct tinywl_server {
	struct wl_display* wl_display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;
	struct wlr_xdg_shell* xdg_shell;

	struct wlr_output_layout* output_layout;
	struct tinywl_output* output;
//	struct wl_list outputs;
	struct wl_listener new_output;
	struct wl_listener new_xdg_surface;
	struct wl_list views;
};

struct tinywl_output {
	struct wl_list link;
	struct tinywl_server* server;
	struct wlr_output* wlr_output;
	struct wl_listener frame;
	FlutterEngine engine;
};

struct tinywl_view {
	struct wl_list link;
	struct tinywl_server* server;
	struct wlr_xdg_surface* xdg_surface;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
	bool mapped;
	int x, y;
};

FlutterEngine RunFlutter(struct tinywl_server* server);

void p(void* userdata) {
	struct tinywl_output* output = userdata;
	struct wlr_renderer* renderer = output->server->renderer;
	EGLContext context = wlr_gles2_renderer_get_egl(renderer)->context;
	EGLDisplay display = wlr_gles2_renderer_get_egl(renderer)->display;


	if (!wlr_output_attach_render(output->wlr_output, NULL)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);
	/* Begin the renderer (calls glViewport and some other GL sanity checks) */
//	pthread_mutex_lock(&vsync_mutex);
	wlr_renderer_begin(renderer, width, height);

//	struct wlr_egl* context;
//	context = wlr_gles2_renderer_get_egl(renderer);
//	bool a = wlr_egl_is_current(context);
//	bool b = wlr_egl_make_current(context);
//	printf("\n%d\n", a);
//	fflush(stdout);
//	printf("\n%d\n", b);
//	fflush(stdout);

	pthread_mutex_lock(&baton_mutex);
	uint64_t start = FlutterEngineGetCurrentTime();
	FlutterEngineOnVsync(output->engine, g_baton, start, start + 1000000000ull / 144);
	pthread_mutex_unlock(&baton_mutex);
}

static void output_frame(struct wl_listener* listener, void* data) {
	printf("\nOUTPUT FRAME\n\n");
	fflush(stdout);

	/* This function is called every time an output is ready to display a frame,
	 * generally at the output's refresh rate (e.g. 60Hz). */
	struct tinywl_output* output =
		  wl_container_of(listener, output, frame);
	struct wlr_renderer* renderer = output->server->renderer;

//	struct timespec now;
//	clock_gettime(CLOCK_MONOTONIC, &now);

	FlutterEnginePostRenderThreadTask(output->engine, p, output);
	sem_wait(&vsync_semaphore);

	/* wlr_output_attach_render makes the OpenGL context current. */
//	flutter_make_current()


//	if (!wlr_output_attach_render(output->wlr_output, NULL)) {
//		return;
//	}
//	/* The "effective" resolution can change if you rotate your outputs. */
//	int width, height;
//	wlr_output_effective_resolution(output->wlr_output, &width, &height);
//
//	wlr_renderer_begin(output->server->renderer, width, height);
//
//	float color[4] = {0.6, 0.3, 0.3, 1.0};
//	wlr_renderer_clear(output->server->renderer, color);
//
////	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
////	glClear(GL_COLOR_BUFFER_BIT);
//
//	wlr_renderer_end(output->server->renderer);
//	wlr_output_commit(output->wlr_output);


}

static void server_new_output(struct wl_listener* listener, void* data) {
	struct tinywl_server* server =
		  wl_container_of(listener, server, new_output);
	struct wlr_output* wlr_output = data;

	if (server->output != NULL) {
		return;
	}

	if (!wl_list_empty(&wlr_output->modes)) {
		struct wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
		wlr_output_set_mode(wlr_output, mode);
		wlr_output_enable(wlr_output, true);

		if (!wlr_output_commit(wlr_output)) {
			return;
		}
	}

	/* Allocates and configures our state for this output */
	struct tinywl_output* output = calloc(1, sizeof(struct tinywl_output));
	output->wlr_output = wlr_output;
	output->server = server;

	output->frame.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);
	server->output = output;
//	wl_list_insert(&server->outputs, &output->link);

	wlr_output_layout_add_auto(server->output_layout, wlr_output);

//	if (!wlr_output_attach_render(output->wlr_output, NULL)) {
//		printf("NOOOO\n");
//		return;
//	}
//
//	/* The "effective" resolution can change if you rotate your outputs. */
	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);
//
//	/* Begin the renderer (calls glViewport and some other GL sanity checks) */
//	wlr_renderer_begin(server->renderer, width, height);
//
//	float color[4] = {0.6, 0.3, 0.3, 1.0};
//	wlr_renderer_clear(server->renderer, color);
//
////	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
////	glClear(GL_COLOR_BUFFER_BIT);
//
//	wlr_renderer_end(server->renderer);
//	wlr_output_commit(output->wlr_output);
//
////	printf("HAHA ");
//
	FlutterWindowMetricsEvent event = {};
	event.struct_size = sizeof(event);
	event.width = width;
	event.height = height;
	event.pixel_ratio = 1.0;
	printf("%zu %zu ", event.width, event.height);

	FlutterEngine engine = RunFlutter(server);
	output->engine = engine;

	FlutterEngineSendWindowMetricsEvent(engine, &event);
}

static void xdg_surface_map(struct wl_listener* listener, void* data) {
	printf("MAP SURFACE\n");
	fflush(stdout);
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct tinywl_view* view = wl_container_of(listener, view, map);
	view->mapped = true;
}

static void xdg_surface_unmap(struct wl_listener* listener, void* data) {
	printf("UNMAP SURFACE\n");
	fflush(stdout);
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct tinywl_view* view = wl_container_of(listener, view, unmap);
	view->mapped = false;
}

static void xdg_surface_destroy(struct wl_listener* listener, void* data) {
	printf("DESTROY SURFACE\n");
	fflush(stdout);
	/* Called when the surface is destroyed and should never be shown again. */
	struct tinywl_view* view = wl_container_of(listener, view, destroy);
	wl_list_remove(&view->link);
	free(view);
}

static void server_new_xdg_surface(struct wl_listener* listener, void* data) {
//	pthread_mutex_lock(&vsync_mutex);

	/* This event is raised when wlr_xdg_shell receives a new xdg surface from a
	 * client, either a toplevel (application window) or popup. */
	struct tinywl_server* server =
		  wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface* xdg_surface = data;
	if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		return;
	}

	/* Allocate a tinywl_view for this surface */
	struct tinywl_view* view =
		  calloc(1, sizeof(struct tinywl_view));
	view->server = server;
	view->xdg_surface = xdg_surface;

	/* Listen to the various events it can emit */
	view->map.notify = xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	/* Add it to the list of views. */
	wl_list_insert(&server->views, &view->link);

	printf("\nNEW CLIENT\n\n");
//	pthread_mutex_unlock(&vsync_mutex);
}

bool flutter_make_current(void* userdata) {
	printf("MAKE_CURRENT\n");
	fflush(stdout);

	struct tinywl_server* server = userdata;
	struct tinywl_output* output;

//	wl_list_for_each(output, &server->outputs, link) {
//		if (output->wlr_output->back_buffer == NULL) {
////			printf("\nF\n");
//			if (!wlr_output_attach_render(output->wlr_output, NULL)) {
//				return false;
//			}
//			int width, height;
//			wlr_output_effective_resolution(output->wlr_output, &width, &height);
//			/* Begin the renderer (calls glViewport and some other GL sanity checks) */
//			wlr_renderer_begin(server->renderer, width, height);
//		}
//	}

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
	printf("CLEAR_CURRENT\n");
	fflush(stdout);
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
	printf("PRESENT\n");
	fflush(stdout);

	struct tinywl_server* server = userdata;
	struct wlr_renderer* renderer = server->renderer;
	wlr_renderer_end(renderer);
	if (!server->output->wlr_output->frame_pending) {
		wlr_output_commit(server->output->wlr_output);
		sem_post(&vsync_semaphore);
	}
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
	FlutterTransformation transformation;

	int width, height;
	wlr_output_effective_resolution(server->output->wlr_output, &width, &height);
	transformation.scaleX = 1;
	transformation.scaleY = -1;
	transformation.pers2 = 1;
	transformation.transY = height;

	return transformation;
}

void vsync_callback(void* userdata, intptr_t baton) {
	printf("\nVSYNC CALLBACK\n\n");
	fflush(stdout);

	pthread_mutex_lock(&baton_mutex);
	g_baton = baton;
	pthread_mutex_unlock(&baton_mutex);
}

FlutterEngine RunFlutter(struct tinywl_server* server) {
	FlutterRendererConfig config = {};
	config.type = kOpenGL;
	config.open_gl.struct_size = sizeof(config.open_gl);
	config.open_gl.make_current = flutter_make_current;
	config.open_gl.clear_current = flutter_clear_current;
	config.open_gl.present = flutter_present;
	config.open_gl.fbo_callback = flutter_fbo_callback;
//	config.open_gl.surface_transformation = flutter_surface_transformation;

#define BUNDLE "build/linux/x64/debug/bundle/data"
	FlutterProjectArgs args = {
		  .struct_size = sizeof(FlutterProjectArgs),
		  .assets_path = BUNDLE "/flutter_assets", // This directory is generated by `flutter build bundle`
		  .icu_data_path = BUNDLE "/icudtl.dat", // Find this in your bin/cache directory.
		  .vsync_callback = vsync_callback,
	};
	FlutterEngine engine = NULL;
	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, // renderer
	                              &args, server, &engine);

	assert(result == kSuccess && engine != NULL);

	//	glfwSetWindowUserPointer(window, engine);

//	GLFWwindowSizeCallback(window, kInitialWindowWidth, kInitialWindowHeight);

	return engine;
}

int main(int argc, const char* argv[]) {
	pthread_mutex_init(&baton_mutex, NULL);
	sem_init(&vsync_semaphore, 0, 0);

	wlr_log_init(WLR_DEBUG, NULL);

	struct tinywl_server server;
	server.output = NULL;
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
	wl_signal_add(&server.xdg_shell->events.new_surface,
	              &server.new_xdg_surface);

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

	/* Set the WAYLAND_DISPLAY environment variable to our socket and run the startup command if requested. */
	setenv("WAYLAND_DISPLAY", socket, true);
	setenv("XDG_SESSION_TYPE", "wayland", true);
	if (fork() == 0) {
		execl("/bin/sh", "/bin/sh", "-c", "kate", (void*) NULL);
	}

	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
	        socket);
	wl_display_run(server.wl_display);

	wl_display_destroy_clients(server.wl_display);
	wl_display_destroy(server.wl_display);

	return EXIT_SUCCESS;
}