#include <cassert>
#include <xf86drm.h>
#include <csignal>
#include "output.hpp"
#include "server.hpp"
#include "embedder_callbacks.hpp"
#include "time.hpp"
#include "render_view.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <future>
#include <sys/eventfd.h>

extern "C" {
#define static
#include <wlr/render/gles2.h>
#include <wlr/util/log.h>
#include <wlr/backend/drm.h>
#undef static
}

ZenithOutput::ZenithOutput(ZenithServer* server, struct wlr_output* wlr_output)
	  : server(server), wlr_output(wlr_output) {

//	wlr_output_lock_software_cursors(wlr_output, true);

	frame_listener.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &frame_listener);
	mode_changed.notify = mode_changed_event;
	wl_signal_add(&wlr_output->events.mode, &mode_changed);

	wlr_output_set_scale(wlr_output, server->display_scale);


	int drm_fd = wlr_backend_get_drm_fd(server->backend);

	vsync_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);

	wl_event_loop* event_loop = wl_display_get_event_loop(server->display);
	wl_event_loop_add_fd(event_loop, vsync_fd, WL_EVENT_READABLE, vblank_handler, this);

//	std::thread([](int drm_fd, int vsync_fd, struct wlr_output* output) {
//		while (true) {
//			drmVBlank vbl = {
//				  .request = {
//						.type = DRM_VBLANK_RELATIVE,
//						.sequence = 1,
//						.signal = 0
//				  },
//			};
//			int ret = drmWaitVBlank(drm_fd, &vbl);
//			std::cout << "write vbl " << FlutterEngineGetCurrentTime() << " " << vbl.reply.sequence << std::endl;
//			if (!output->frame_pending) {
////				eventfd_write(vsync_fd, 1);
//			}
//			if (ret != 0) {
//				break;
//			}
//		}
//	}, drm_fd, vsync_fd, wlr_output).detach();

//	drm_epoll_fd = epoll_create1(0);
//
//	epoll_event ev = {
//		  .events = EPOLLIN | EPOLLOUT,
//		  .data = {.fd = drm_fd},
//	};
//	int r = epoll_ctl(drm_epoll_fd, EPOLL_CTL_ADD, drm_fd, &ev);
//
//
////	struct epoll_event evnts[30];
////	int count = epoll_wait(drm_epoll_fd, evnts, 30, -1);
//
//	wl_event_loop* event_loop = wl_display_get_event_loop(server->display);
//	auto a = wl_event_loop_add_fd(event_loop, drm_fd, WL_EVENT_READABLE, drm_event, this);

//	std::cout << "FD " << a << std::endl;
}

static int i = 0;

void output_frame(wl_listener* listener, void* data) {
//	std::cout << "page flip " << FlutterEngineGetCurrentTime() << std::endl;

	ZenithOutput* output = wl_container_of(listener, output, frame_listener);
	ZenithServer* server = output->server;
	vsync_callback(server);
//	if (i == 0) {
//		wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
//		if (wlr_egl_is_current(egl)) {
//			wlr_egl_unset_current(egl);
//		}
//	}

	auto& flutter_engine_state = server->embedder_state;

	uint64_t now = current_time_nanoseconds();

//	if (!wlr_egl_is_current(egl)) {
//		wlr_egl_make_current(egl);
//	}
//
//	if (output->wlr_output->back_buffer != nullptr) {
//		wlr_output_rollback(output->wlr_output);
//	}

//	int width = server->output->wlr_output->width;
//	int height = server->output->wlr_output->height;
//	wlr_renderer_begin(server->renderer, width, height);
//
//	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(server->renderer);
//	std::cout << output_fbo << std::endl;
//
//	zenith_egl_restore_context(&saved_egl_context);

//	wlr_egl* egl = wlr_gles2_renderer_get_egl(server->renderer);
//	wlr_egl_make_current(egl);

	for (auto& [id, view]: server->views) {
		if (!view->mapped || !view->visible || wlr_surface_get_texture(view->xdg_surface->surface) == nullptr) {
			// An unmapped view should not be rendered.
			continue;
		}

		std::shared_ptr<Framebuffer> view_framebuffer;

		{
			std::scoped_lock lock(server->surface_framebuffers_mutex);

			auto surface_framebuffer_it = server->surface_framebuffers.find(view->active_texture);
			assert(surface_framebuffer_it != server->surface_framebuffers.end());

			view_framebuffer = surface_framebuffer_it->second;
		}

		std::scoped_lock lock(view_framebuffer->mutex);

		GLuint view_fbo = view_framebuffer->framebuffer;
		render_view_to_framebuffer(view, view_fbo);

		FlutterEngineMarkExternalTextureFrameAvailable(flutter_engine_state->engine, (int64_t) view->active_texture);
	}

//	wlr_output_schedule_frame(server->output->wlr_output);

//	std::cout << "vsync " << current_time_nanoseconds() << std::endl;

//	if (i == 0) {
//		if (!wlr_output_attach_render(server->output->wlr_output, nullptr)) {
//			return;
//		}
//		GLint fb;
//		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
//		if (!wlr_output_attach_render(server->output->wlr_output, nullptr)) {
//			return;
//		}
//	}
//		server->fb_channel.write(fb);
//	} else {
//		if (!wlr_output_attach_render(server->output->wlr_output, nullptr)) {
//			std::cerr << "attach failed";
//			return;
//		}
//
//		GLint fb;
//		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
//
//		server->fb_channel.write(fb);
//	}

//	{
//		/*
//		 * Notify the compositor to prepare a new frame for the next time.
//		 */
//		std::scoped_lock lock(flutter_engine_state->baton_mutex);
//
//		if (flutter_engine_state->new_baton) {
//			intptr_t baton = flutter_engine_state->baton;
//			flutter_engine_state->new_baton = false;
//
//			double refresh_rate = output->wlr_output->refresh != 0
//			                      ? (double) output->wlr_output->refresh / 1000
//			                      : 60; // Suppose it's 60Hz if the refresh rate is not available.
//
//			uint64_t next_frame = now + (uint64_t) (1'000'000'000ull / refresh_rate);
//			FlutterEngineOnVsync(flutter_engine_state->engine, baton, now, next_frame);
//		}
//	}

	if (i == 0) {


//		wlr_output_schedule_frame(output->wlr_output);
		i++;
	}

//	std::cout << "frame" << std::endl;

	/*
	 * Copy the frame to the screen.
	 */
//	if (!wlr_output_attach_render(output->wlr_output, nullptr)) {
//		return;
//	}
//
//	int width = output->wlr_output->width;
//	int height = output->wlr_output->height;
//	wlr_renderer_begin(server->renderer, width, height);
//
//	uint32_t output_fbo = wlr_gles2_renderer_get_current_fbo(server->renderer);
//
//	{
//		Framebuffer& copy_framebuffer = *flutter_engine_state->output_framebuffer;
//		std::scoped_lock lock(copy_framebuffer.mutex);
//		GLScopedLock gl_lock(flutter_engine_state->output_gl_mutex);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
//		glClear(GL_COLOR_BUFFER_BIT);
//		RenderToTextureShader::instance()->render(copy_framebuffer.texture, 0, 0, copy_framebuffer.width,
//		                                          copy_framebuffer.height, output_fbo, true);
//	}

//	wlr_output_rollback(output->wlr_output);

	/* Hardware cursors are rendered by the GPU on a separate plane, and can be
	 * moved around without re-rendering what's beneath them - which is more
	 * efficient. However, not all hardware supports hardware cursors. For this
	 * reason, wlroots provides a software fallback, which we ask it to render
	 * here. wlr_cursor handles configuring hardware vs software cursors for you,
	 * and this function is a no-op when hardware cursors are in use. */
//	wlr_output_render_software_cursors(output->wlr_output, nullptr);
//
//	wlr_renderer_end(server->renderer);

	// The output might be disabled. Cancel the operation if the output is not ready.

//	if (!wlr_output_test(output->wlr_output)) {
//		wlr_output_rollback(output->wlr_output);
//		return;
//	}

	// FIXME:
	// Sometimes, committing a new frame to the screen just fails. I suspect it's because
	// this function takes too long to render everything and we miss the vblank period.
	// The rendering should be pretty fast because we're just copying some textures, but I think
	// implicit synchronization between the compositor and Wayland clients is the issue here.
	// Normally, this callback is automatically called every frame, but when this happens, it stops
	// being called. To avoid having a frozen screen, we manually schedule the next frame.
//	if (!wlr_output_commit(output->wlr_output)) {
//		wlr_output_schedule_frame(output->wlr_output);
//	}
}

void mode_changed_event(wl_listener* listener, void* data) {
	ZenithOutput* output = wl_container_of(listener, output, mode_changed);

	// Update the layout box when the mode changes.
	wlr_box* box = wlr_output_layout_get_box(output->server->output_layout, nullptr);
	output->server->output_layout_box = *box;

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);

	FlutterWindowMetricsEvent window_metrics = {};
	window_metrics.struct_size = sizeof(FlutterWindowMetricsEvent);
	window_metrics.width = output->wlr_output->width;
	window_metrics.height = output->wlr_output->height;
	window_metrics.pixel_ratio = output->server->display_scale;

	wlr_egl_make_current(wlr_gles2_renderer_get_egl(output->server->renderer));
	output->server->embedder_state->send_window_metrics(window_metrics);
}

int drm_event(int fd, uint32_t mask, void* data) {
	auto* output = static_cast<ZenithOutput*>(data);
//
	drmEventContext event = {
		  .version = 3,
		  .page_flip_handler2 = page_flip_handler2,
//		  .page_flip_handler = nullptr,
//		  .page_flip_handler2 = nullptr,
//		  .sequence_handler = nullptr,
	};

	if (drmHandleEvent(fd, &event) != 0) {
		wlr_log(WLR_ERROR, "drmHandleEvent failed");
		wl_display_terminate(output->server->display);
	}

	return 0;
}

int vblank_handler(int fd, uint32_t mask, void* data) {
	auto* output = static_cast<ZenithOutput*>(data);
	auto* server = output->server;
	auto& flutter_engine_state = server->embedder_state;
	uint64_t now = current_time_nanoseconds();

	eventfd_t tmp;
	eventfd_read(fd, &tmp);

	{
		/*
		 * Notify the compositor to prepare a new frame for the next time.
		 */
		std::scoped_lock lock(flutter_engine_state->baton_mutex);

		if (flutter_engine_state->new_baton) {
			intptr_t baton = flutter_engine_state->baton;
			flutter_engine_state->new_baton = false;

			double refresh_rate = output->wlr_output->refresh != 0
			                      ? (double) output->wlr_output->refresh / 1000
			                      : 60; // Suppose it's 60Hz if the refresh rate is not available.

			uint64_t next_frame = now + (uint64_t) (1'000'000'000ull / refresh_rate);
			FlutterEngineOnVsync(flutter_engine_state->engine, baton, now, next_frame);
		}
	}

	return 0;
}

void page_flip_handler2(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, unsigned int crtc_id,
                        void* user_data) {
	std::cout << "PAGE FLIP " << sequence << std::endl;

}
