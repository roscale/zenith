#include <iostream>
#include "output_callbacks.hpp"
#include "embedder.h"
#include "flutland_structs.hpp"
#include "flutter_callbacks.hpp"
#include "platform_channels/event_channel.h"
#include "create_shared_egl_context.hpp"
#include <src/platform_channels/event_stream_handler_functions.h>

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
#undef static
}

void server_new_output(struct wl_listener* listener, void* data) {
	struct flutland_server* server = wl_container_of(listener, server, new_output);
	struct wlr_output* wlr_output = static_cast<struct wlr_output*>(data);

	if (server->output != nullptr) {
		// Allow only one output at the moment.
		return;
	}

	if (!wl_list_empty(&wlr_output->modes)) {
		wlr_output_enable(wlr_output, true);
		struct wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
		wlr_output_set_mode(wlr_output, mode);

		if (!wlr_output_commit(wlr_output)) {
			return;
		}
	}

	/* Allocates and configures our state for this output */
	struct flutland_output* output = static_cast<flutland_output*>(calloc(1, sizeof(struct flutland_output)));
	output->wlr_output = wlr_output;
	output->server = server;

	pthread_mutex_init(&output->baton_mutex, nullptr);
	sem_init(&output->vsync_semaphore, 0, 0);

	output->frame.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);
	server->output = output;

	wlr_output_layout_add_auto(server->output_layout, wlr_output);

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);


	struct wlr_egl* egl = wlr_gles2_renderer_get_egl(output->server->renderer);
	output->platform_thread_egl_context = create_shared_egl_context(egl);
	std::cout << "SHARED CONTEXT " << output->platform_thread_egl_context << std::endl;

	wlr_egl_make_current(output->platform_thread_egl_context);
	output->fix_y_flip_state = fix_y_flip_init_state(width, height);
	wlr_egl_unset_current(output->platform_thread_egl_context);


	FlutterWindowMetricsEvent event = {};
	event.struct_size = sizeof(event);
	event.width = width;
	event.height = height;
	event.pixel_ratio = 1.0;
	printf("%zu %zu ", event.width, event.height);

	FlutterEngine engine = run_flutter(output);
	output->engine = engine;
	output->messenger = BinaryMessenger(engine);
	output->message_dispatcher = IncomingMessageDispatcher(&output->messenger);
	output->messenger.SetMessageDispatcher(&output->message_dispatcher);

//	output->new_texture_event_channel = std::make_unique<flutter::EventChannel<>>(&output->messenger, "new_texture_id",
//	                                                                              &flutter::StandardMethodCodec::GetInstance());
//	output->new_texture_stream_handler = std::make_unique<NewTextureStreamHandler<>>();
//	output->new_texture_event_channel->SetStreamHandler(output->new_texture_stream_handler);
//	output->new_texture_event_channel->SetStreamHandler()


	// This handler is necessary because flutter expects to be able to call listen() and cancel() on the platform even
	// though we don't care because this channel will always be open.
	auto &codec = flutter::StandardMethodCodec::GetInstance();

	output->window_mapped_event_channel = std::make_unique<flutter::EventChannel<>>(&output->messenger, "window_mapped",
	                                                                                &codec);
	output->window_mapped_event_channel->SetStreamHandler(
			std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
					[](
							const flutter::EncodableValue* arguments,
							std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events)
							-> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {

						std::cout << "LISTENING WINDOW_MAPPED" << std::endl;
						return nullptr;
					},
					[](const flutter::EncodableValue* arguments)
							-> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {

						return nullptr;
					}
			));

	output->window_unmapped_event_channel = std::make_unique<flutter::EventChannel<>>(&output->messenger,
	                                                                                  "window_unmapped", &codec);
	output->window_unmapped_event_channel->SetStreamHandler(
			std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
					[](
							const flutter::EncodableValue* arguments,
							std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events)
							-> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {

						std::cout << "LISTENING WINDOW_UNMAPPED" << std::endl;
						return nullptr;
					},
					[](const flutter::EncodableValue* arguments)
							-> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {

						return nullptr;
					}
			));

	output->platform_method_channel = std::make_unique<flutter::MethodChannel<>>(&output->messenger,
	                                                                             "platform", &codec);

	output->platform_method_channel->SetMethodCallHandler(
			[](const flutter::MethodCall<> &call, std::unique_ptr<flutter::MethodResult<>> result) {
				if (call.method_name() == "activate_window") {
					int64_t view_ptr_int = std::get<int64_t>(call.arguments()[0]);
					auto* view = reinterpret_cast<flutland_view*>(view_ptr_int);

					if (view->server->active_view == view) {
						// View already active. Noop.
						result->Success();
						return;
					}

					if (view->server->active_view != nullptr) {
						wlr_xdg_toplevel_set_activated(view->server->active_view->xdg_surface, false);
					}

					wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
					view->server->active_view = view;

					result->Success();
					return;
				}
				result->Error("method_does_not_exist", "Method " + call.method_name() + " does not exist");
			});

	FlutterEngineSendWindowMetricsEvent(engine, &event);
}

void output_frame(struct wl_listener* listener, void* data) {
//	std::clog << "OUTPUT FRAME" << std::endl;

	/* This function is called every time an output is ready to display a frame,
	 * generally at the output's refresh rate (e.g. 60Hz). */
	struct flutland_output* output = wl_container_of(listener, output, frame);

	while (true) {
		pthread_mutex_lock(&output->baton_mutex);
		if (output->new_baton) {
			pthread_mutex_unlock(&output->baton_mutex);
			break;
		}
		pthread_mutex_unlock(&output->baton_mutex);
	}
//	pthread_mutex_lock(&output->baton_mutex);
//	while (!output->new_baton);
//	if (!output->new_baton) {
//		pthread_mutex_unlock(&output->baton_mutex);
//		return;
//	}
//	pthread_mutex_unlock(&output->baton_mutex);

	struct flutland_view* view;
	wl_list_for_each_reverse(view, &output->server->views, link) {
		if (!view->mapped) {
			/* An unmapped view should not be rendered. */
			continue;
		}
		struct wlr_texture* texture = wlr_surface_get_texture(view->xdg_surface->surface);
		FlutterEngineMarkExternalTextureFrameAvailable(output->engine, (int64_t) texture);

		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		wlr_surface_send_frame_done(view->xdg_surface->surface, &now);
	}

	// Rendering can only be started on the flutter render thread because the context is current on that thread.
	FlutterEnginePostRenderThreadTask(output->engine, start_rendering, output);

	pthread_mutex_lock(&output->baton_mutex);
	intptr_t baton = output->baton;
	output->new_baton = false;
	pthread_mutex_unlock(&output->baton_mutex);

//	output->new_baton = false;
	uint64_t start = FlutterEngineGetCurrentTime();
//	std::clog << "BATON "<< baton << std::endl;
	FlutterEngineOnVsync(output->engine, baton, start, start + 1000000000ull / 144);


	sem_wait(&output->vsync_semaphore);

	// Execute all platform tasks while waiting for the next frame event.
	wl_event_loop_add_idle(wl_display_get_event_loop(output->server->wl_display), flutter_execute_platform_tasks,
	                       nullptr);
}
