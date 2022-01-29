#include <cassert>
#include "output.hpp"
#include "server.hpp"
#include "flutter_callbacks.hpp"

extern "C" {
#define static
#include <wlr/render/gles2.h>
#undef static
}

static void render_view_to_framebuffer(ZenithView* view, GLuint view_fbo);

struct render_data {
	ZenithView* view;
	GLuint view_fbo;
};

ZenithOutput::ZenithOutput(ZenithServer* server, struct wlr_output* wlr_output)
	  : server(server), wlr_output(wlr_output) {

	frame_listener.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &frame_listener);
	mode_changed.notify = mode_changed_event;
	wl_signal_add(&wlr_output->events.mode, &mode_changed);
}

void output_frame(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	ZenithServer* server = output->server;
	auto& flutter_engine_state = server->flutter_engine_state;

	wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
	wlr_egl_make_current(egl);

	for (auto& pair: server->views_by_id) {
		size_t view_id = pair.first;
		std::unique_ptr<ZenithView>& view = pair.second;

		if (!view->mapped || wlr_surface_get_texture(view->xdg_surface->surface) == nullptr) {
			// An unmapped view should not be rendered.
			continue;
		}

		std::shared_ptr<SurfaceFramebuffer> view_framebuffer;

		{
			std::scoped_lock lock(server->surface_framebuffers_mutex);

			auto surface_framebuffer_it = server->surface_framebuffers.find(view->id);
			assert(surface_framebuffer_it != server->surface_framebuffers.end());

			view_framebuffer = surface_framebuffer_it->second;
		}

		std::scoped_lock lock(view_framebuffer->mutex);

		GLuint view_fbo = view_framebuffer->framebuffer;
		render_view_to_framebuffer(view.get(), view_fbo);

		FlutterEngineMarkExternalTextureFrameAvailable(flutter_engine_state->engine, (int64_t) view_id);
	}

	{
		/*
		 * Notify the compositor to prepare a new frame for the next time.
		 */
		std::scoped_lock lock(flutter_engine_state->baton_mutex);

		if (flutter_engine_state->new_baton) {
			intptr_t baton = flutter_engine_state->baton;
			flutter_engine_state->new_baton = false;

			uint64_t start = FlutterEngineGetCurrentTime();
			FlutterEngineOnVsync(flutter_engine_state->engine, baton, start, start + 1'000'000'000ull / 144);
		}
	}

	/*
	 * Copy the frame to the screen.
	 */
	if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);
	wlr_renderer_begin(server->renderer, width, height);

	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(server->renderer);

	{
		std::scoped_lock lock(flutter_engine_state->present_fbo->mutex);
		GLScopedLock gl_lock(flutter_engine_state->gl_mutex);

		glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		RenderToTextureShader::instance()->render(flutter_engine_state->present_fbo->texture, 0, 0, width,
		                                          height, output_fbo);
	}

	/* Hardware cursors are rendered by the GPU on a separate plane, and can be
	 * moved around without re-rendering what's beneath them - which is more
	 * efficient. However, not all hardware supports hardware cursors. For this
	 * reason, wlroots provides a software fallback, which we ask it to render
	 * here. wlr_cursor handles configuring hardware vs software cursors for you,
	 * and this function is a no-op when hardware cursors are in use. */
	wlr_output_render_software_cursors(output->wlr_output, nullptr);

	wlr_renderer_end(server->renderer);
	wlr_output_commit(output->wlr_output);

	if (server->pointer->kinetic_scrolling) {
		uint32_t now = FlutterEngineGetCurrentTime() / 1'000'000;
		double average_delta_x = server->pointer->average_delta_x;
		double average_delta_y = server->pointer->average_delta_y;

		uint32_t last_real_event = server->pointer->last_real_event;

		double sum_x = 0;
		double sum_y = 0;
		const double length = 150;

		for (uint32_t ms = server->pointer->last_kinetic_event_time; ms <= now; ms += 1) {
			const double amplitude_x = 1.4 * abs(average_delta_x);
			const double amplitude_y = 1.4 * abs(average_delta_y);

			auto vel_x = amplitude_x * exp(-(double) (ms - last_real_event) / length);
			auto vel_y = amplitude_y * exp(-(double) (ms - last_real_event) / length);

			auto value_x = vel_x / 10;
			auto value_y = vel_y / 10;

			if (average_delta_x < 0) {
				value_x *= -1;
			}
			sum_x += value_x;

			if (average_delta_y < 0) {
				value_y *= -1;
			}
			sum_y += value_y;

			if (vel_x * vel_x + vel_y * vel_y < 0.1 * 0.1) {
				server->pointer->kinetic_scrolling = false;
				break;
			}
		}

		auto event = &server->pointer->last_real_scroll_event;

		wlr_seat_pointer_notify_axis(server->seat,
		                             now, WLR_AXIS_ORIENTATION_HORIZONTAL, sum_x,
		                             0, event->source);
		wlr_seat_pointer_notify_frame(server->seat);
		wlr_seat_pointer_notify_axis(server->seat,
		                             now, WLR_AXIS_ORIENTATION_VERTICAL, sum_y,
		                             0, event->source);
		wlr_seat_pointer_notify_frame(server->seat);
		if (not server->pointer->kinetic_scrolling) {
			wlr_seat_pointer_notify_axis(server->seat,
			                             now, WLR_AXIS_ORIENTATION_HORIZONTAL, 0,
			                             0, event->source);
			wlr_seat_pointer_notify_frame(server->seat);
			wlr_seat_pointer_notify_axis(server->seat,
			                             now, WLR_AXIS_ORIENTATION_VERTICAL, 0,
			                             0, event->source);
			wlr_seat_pointer_notify_frame(server->seat);
		}

		server->pointer->last_kinetic_event_time = now;

//		if (vel >= 0.1) {
//			std::cout << "scroll " << incr << std::endl;
//			wlr_seat_server->pointer_notify_axis(server->seat,
//			                             now, event->orientation, vel,
//			                             event->delta_discrete, event->source);
//			wlr_seat_server->pointer_notify_frame(server->seat);
//			server->pointer->last_kinetic_event = now;
//		} else {
//			std::cout << "scroll end" << std::endl;
//			server->pointer->kinetic_scrolling = false;
//		}

//		std::cout << server->pointer->last_kinetic_event << std::endl;
//		std::cout << now << std::endl;
//		for (uint32_t t = server->pointer->last_kinetic_event; t <= now; t += 1) {
//			if (abs(event->delta) >= 0.1) {
//				event->delta *= 0.995;
//			} else {
//				server->pointer->kinetic_scrolling = false;
//				break;
//			}
//		}
//		wlr_seat_server->pointer_notify_axis(server->seat,
//		                             now, event->orientation, event->delta,
//		                             event->delta_discrete, event->source);
//		wlr_seat_server->pointer_notify_frame(server->seat);
//		server->pointer->last_kinetic_event = now;
	}

}

void mode_changed_event(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, mode_changed);

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = width;
	window_metrics.height = height;
	window_metrics.pixel_ratio = 1.0;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));
	output->server->flutter_engine_state->send_window_metrics(window_metrics);
}

static void render_view_to_framebuffer(ZenithView* view, GLuint view_fbo) {
	glBindFramebuffer(GL_FRAMEBUFFER, view_fbo);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	render_data rdata = {
		  .view = view,
		  .view_fbo = view_fbo,
	};

	try {
		// Render the subtree of subsurfaces starting from a toplevel or popup.
		wlr_xdg_surface_for_each_surface(
			  view->xdg_surface,
			  [](struct wlr_surface* surface, int sx, int sy, void* data) {
				  auto* rdata = static_cast<render_data*>(data);
				  auto* view = rdata->view;

				  if (surface != view->xdg_surface->surface && wlr_surface_is_xdg_surface(surface)) {
					  // Don't render child popups. They will be rendered separately on their own framebuffer,
					  // so skip this subtree of surfaces.
					  // There's no easy way to exit this recursive iterator, but we can still do this by throwing a
					  // dummy exception.
					  throw std::exception();
				  }

				  wlr_texture* texture = wlr_surface_get_texture(surface);
				  if (texture == nullptr) {
					  // I don't remember if it's possible to have a mapped surface without texture, but better to be
					  // safe than sorry. Just skip it.
					  return;
				  }

				  wlr_gles2_texture_attribs attribs{};
				  wlr_gles2_texture_get_attribs(texture, &attribs);

				  if (wlr_surface_is_xdg_surface(surface)) {
					  // xdg_surfaces each have their own framebuffer, so they are always drawn at (0, 0).
					  RenderToTextureShader::instance()->render(attribs.tex, 0, 0,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_fbo);
				  } else {
					  // This is true when the surface is a wayland subsurface and not an xdg_surface.
					  // I decided to render subsurfaces on the framebuffer of the nearest xdg_surface parent.
					  // One downside to this approach is that popups created using a subsurface instead of an xdg_popup
					  // are clipped to the window and cannot be rendered outside the window bounds.
					  // I don't really care because Gnome Shell takes the same approach, and it simplifies things a lot.
					  // Interesting discussion: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/24
					  RenderToTextureShader::instance()->render(attribs.tex, sx, sy,
					                                            surface->current.buffer_width,
					                                            surface->current.buffer_height,
					                                            rdata->view_fbo);
				  }

				  // Tell the client we are done rendering this surface.
				  timespec now{};
				  clock_gettime(CLOCK_MONOTONIC, &now);
				  wlr_surface_send_frame_done(surface, &now);
			  },
			  &rdata
		);
	} catch (std::exception& unused) {
	}
}