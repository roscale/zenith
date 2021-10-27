#include "flutter_callbacks.h"
#include "flutland_structs.h"

#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_output.h>

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#define BUNDLE "build/linux/x64/debug/bundle/data"

FlutterEngine run_flutter(struct flutland_output* output) {
	FlutterRendererConfig config = {};
	config.type = kOpenGL;
	config.open_gl.struct_size = sizeof(config.open_gl);
	config.open_gl.make_current = flutter_make_current;
	config.open_gl.clear_current = flutter_clear_current;
	config.open_gl.present = flutter_present;
	config.open_gl.fbo_callback = flutter_fbo_callback;

	FlutterProjectArgs args = {
		  .struct_size = sizeof(FlutterProjectArgs),
		  .assets_path = BUNDLE "/flutter_assets",
		  .icu_data_path = BUNDLE "/icudtl.dat",
		  .vsync_callback = vsync_callback,
	};

	FlutterEngine engine = NULL;
	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, output, &engine);
	assert(result == kSuccess && engine != NULL);

	return engine;
}

bool flutter_make_current(void* userdata) {
	printf("MAKE_CURRENT\n");
	fflush(stdout);

	struct flutland_output* output = userdata;

	return wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));
}

bool flutter_clear_current(void* userdata) {
	printf("CLEAR_CURRENT\n");
	fflush(stdout);
	struct flutland_output* output = userdata;

	return wlr_egl_unset_current(wlr_gles2_renderer_get_egl(output->server->renderer));
}

bool flutter_present(void* userdata) {
	printf("PRESENT\n");
	fflush(stdout);

	struct flutland_output* output = userdata;
	struct wlr_renderer* renderer = output->server->renderer;
	wlr_renderer_end(renderer);
	if (!output->wlr_output->frame_pending) {
		wlr_output_commit(output->wlr_output);
		sem_post(&output->vsync_semaphore);
	}
	return true;
}

uint32_t flutter_fbo_callback(void* userdata) {
	struct flutland_output* output = userdata;
	uint32_t fbo = wlr_gles2_renderer_get_current_fbo(output->server->renderer);
	return fbo;
}

void vsync_callback(void* userdata, intptr_t baton) {
	struct flutland_output* output = userdata;
	printf("\nVSYNC CALLBACK\n\n");
	fflush(stdout);

	pthread_mutex_lock(&output->baton_mutex);
	output->baton = baton;
	pthread_mutex_unlock(&output->baton_mutex);
}

void start_rendering(void* userdata) {
	struct flutland_output* output = userdata;
	struct wlr_renderer* renderer = output->server->renderer;

	if (!wlr_output_attach_render(output->wlr_output, NULL)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	wlr_renderer_begin(renderer, width, height);

	pthread_mutex_lock(&output->baton_mutex);
	uint64_t start = FlutterEngineGetCurrentTime();
	FlutterEngineOnVsync(output->engine, output->baton, start, start + 1000000000ull / 144);
	pthread_mutex_unlock(&output->baton_mutex);
}
