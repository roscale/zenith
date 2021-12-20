#include <iostream>
#include "output_callbacks.hpp"
#include "embedder.h"
#include "zenith_structs.hpp"
#include "flutter_callbacks.hpp"
#include "platform_channels/event_channel.h"
#include "platform_methods.hpp"
#include "create_shared_egl_context.hpp"
#include <src/platform_channels/event_stream_handler_functions.h>
#include <cstring>

extern "C" {
#include <semaphore.h>
#include <malloc.h>
#include <pthread.h>
#define static
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_matrix.h>
#undef static
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

	// Allocates and configures our state for this output.

	auto* output = new ZenithOutput(BinaryMessenger(nullptr), IncomingMessageDispatcher(nullptr));
//	auto* output = static_cast<ZenithOutput*>(calloc(1, sizeof(ZenithOutput)));
	output->server = server;
	server->output = output;

	output->wlr_output = wlr_output;

	pthread_mutex_init(&output->baton_mutex, nullptr);
	sem_init(&output->vsync_semaphore, 0, 0);

	output->frame_listener.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame_listener);

	wlr_output_layout_add_auto(server->output_layout, wlr_output);

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	wlr_egl* egl = wlr_gles2_renderer_get_egl(output->server->renderer);

	wlr_egl_make_current(egl);
	output->flutter_gl_context = create_shared_egl_context(egl);
	output->flutter_resource_gl_context = create_shared_egl_context(egl);
	wlr_egl_unset_current(egl);

	// FBOs cannot be shared between GL contexts. Since these FBOs will be used by a Flutter thread, create these
	// resources using the Flutter's context.
	wlr_egl_make_current(output->flutter_gl_context);
	output->fix_y_flip = fix_y_flip_init_state(width, height);
	output->present_fbo = std::make_unique<SurfaceFramebuffer>(width, height);
	wlr_egl_unset_current(output->flutter_gl_context);

	wlr_egl_make_current(egl);
	output->render_to_texture_shader = std::make_unique<RenderToTextureShader>();
	// The Flutter engine will use the wlr renderer and its egl context on the render thread, therefore it must be
	// unset in this thread because a context cannot be bound to multiple threads at once.

	// Create the Flutter engine for this output.
	FlutterEngine engine = run_flutter(output);
	output->engine = engine;
	output->messenger = BinaryMessenger(engine);
	output->message_dispatcher = IncomingMessageDispatcher(&output->messenger);
	output->messenger.SetMessageDispatcher(&output->message_dispatcher);

	auto& codec = flutter::StandardMethodCodec::GetInstance();

	output->platform_method_channel = std::make_unique<flutter::MethodChannel<>>(
		  &output->messenger, "platform", &codec);

	output->platform_method_channel->SetMethodCallHandler(
		  [output](const flutter::MethodCall<>& call, std::unique_ptr<flutter::MethodResult<>> result) {
			  if (call.method_name() == "activate_window") {
				  activate_window(output, call, std::move(result));
			  } else if (call.method_name() == "pointer_hover") {
				  pointer_hover(output, call, std::move(result));
			  } else if (call.method_name() == "pointer_exit") {
				  pointer_exit(output, call, std::move(result));
			  } else if (call.method_name() == "close_window") {
				  close_window(output, call, std::move(result));
			  } else {
				  result->Error("method_does_not_exist", "Method " + call.method_name() + " does not exist");
			  }
		  });

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = width;
	window_metrics.height = height;
	window_metrics.pixel_ratio = 1.0;

	FlutterEngineSendWindowMetricsEvent(engine, &window_metrics);
}

struct render_data {
	ZenithOutput* output;
	ZenithView* view;
	GLuint view_framebuffer;
	bool skip_surface;
};

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);

	wlr_egl* egl = wlr_gles2_renderer_get_egl(output->server->renderer);
	wlr_egl_make_current(egl);

	for (auto pair: output->server->views_by_id) {
		size_t view_id = pair.first;
		ZenithView* view = pair.second;

		if (!view->mapped) {
			/* An unmapped view should not be rendered. */
			continue;
		}

		output->server->surface_framebuffers_mutex.lock();

		GLuint view_framebuffer;
		auto surface_framebuffer_it = output->server->surface_framebuffers.find(view->id);
		if (surface_framebuffer_it == output->server->surface_framebuffers.end()) {
			wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);

			auto inserted_pair = output->server->surface_framebuffers.insert(
				  std::pair<size_t, std::unique_ptr<SurfaceFramebuffer>>(
						view->id,
						std::make_unique<SurfaceFramebuffer>(texture->width, texture->height)
				  )
			);
			surface_framebuffer_it = inserted_pair.first;
		}
		view_framebuffer = surface_framebuffer_it->second->framebuffer;

		output->server->surface_framebuffers_mutex.unlock();

		glClearColor(0, 0, 0, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, view_framebuffer);
		glClear(GL_COLOR_BUFFER_BIT);

		render_data rdata = {
			  .output = output,
			  .view = view,
			  .view_framebuffer = view_framebuffer,
			  .skip_surface = false,
		};

		// Render the subtree of subsurfaces starting from a toplevel or popup.
		wlr_xdg_surface_for_each_surface(
			  view->xdg_surface,
			  [](struct wlr_surface* surface, int sx, int sy, void* data) {
				  auto* rdata = static_cast<render_data*>(data);
				  auto* output = rdata->output;
				  auto* view = rdata->view;

				  if (rdata->skip_surface) {
					  return;
				  }

				  if (surface != view->xdg_surface->surface && wlr_surface_is_xdg_surface(surface)) {
					  // The tree of surfaces might also contain other xdg_surfaces, most notably popups.
					  // Once such surface is encountered, skip it and its surfaces because we are going to draw them
					  // in a separate pass on their own framebuffer. There's no easy way to just break out of this recursive iterator, so we'll
					  // do it using this boolean.
					  rdata->skip_surface = true;
					  return;
				  }

				  wlr_texture* texture = wlr_surface_get_texture(surface);
				  if (texture == nullptr) {
					  return;
				  }

				  wlr_gles2_texture_attribs attribs{};
				  wlr_gles2_texture_get_attribs(texture, &attribs);

				  if (wlr_surface_is_xdg_surface(surface)) {
					  // xdg_surfaces each have their own framebuffer, so they are always drawn at (0, 0) in this framebuffer.
					  output->render_to_texture_shader->render(attribs.tex, 0, 0, surface->current.width,
					                                           surface->current.height,
					                                           rdata->view_framebuffer);
				  } else {
					  // This is true when the surface is a wayland subsurface and not an xdg_surface.
					  // I decided to render subsurfaces on the framebuffer of the nearest xdg_surface parent.
					  // One downside to this approach is that popups created using a subsurface instead of an xdg_popup
					  // are clipped to the window and cannot be rendered outside the window bounds.
					  // I don't really care because Gnome Shell takes the same approach on Ubuntu, and it simplifies things a lot.
					  // Interesting discussion: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/24
					  output->render_to_texture_shader->render(attribs.tex, sx, sy,
					                                           surface->current.width, surface->current.height,
					                                           rdata->view_framebuffer);
				  }

				  // Tell the client we are done rendering this surface.
				  timespec now;
				  clock_gettime(CLOCK_MONOTONIC, &now);
				  wlr_surface_send_frame_done(surface, &now);
			  },
			  &rdata
		);

		FlutterEngineMarkExternalTextureFrameAvailable(output->engine, (int64_t) view_id);
	}

	intptr_t baton;
	pthread_mutex_lock(&output->baton_mutex);
	if (output->new_baton) {
		baton = output->baton;
		output->new_baton = false;

		uint64_t start = FlutterEngineGetCurrentTime();
		FlutterEngineOnVsync(output->engine, baton, start, start + 1'000'000'000ull / 60);
	}
	pthread_mutex_unlock(&output->baton_mutex);

	if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	wlr_renderer_begin(output->server->renderer, width, height);

	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(output->server->renderer);

	output->flip_mutex.lock();
	glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	output->render_to_texture_shader->render(output->present_fbo->texture, 0, 0, width, height, output_fbo);
	output->flip_mutex.unlock();

	/* Hardware cursors are rendered by the GPU on a separate plane, and can be
	 * moved around without re-rendering what's beneath them - which is more
	 * efficient. However, not all hardware supports hardware cursors. For this
	 * reason, wlroots provides a software fallback, which we ask it to render
	 * here. wlr_cursor handles configuring hardware vs software cursors for you,
	 * and this function is a no-op when hardware cursors are in use. */
	wlr_output_render_software_cursors(output->wlr_output, nullptr);

	wlr_renderer_end(output->server->renderer);
	wlr_output_commit(output->wlr_output);

	// Execute all platform tasks while waiting for the next frame event.
	wl_event_loop_add_idle(wl_display_get_event_loop(output->server->display), flutter_execute_platform_tasks,
	                       nullptr);
}
