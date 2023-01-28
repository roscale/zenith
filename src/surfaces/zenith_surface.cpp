#include <sys/ioctl.h>
#include <xf86drm.h>
#include <unistd.h>
#include "zenith_surface.hpp"
#include "server.hpp"
#include "util/wlr/xdg_surface_get_visible_bounds.hpp"
#include "util/wlr/scoped_wlr_buffer.hpp"
#include "util/egl/egl_extensions.hpp"
#include <EGL/eglext.h>

extern "C" {
#include "dma-buf.h"

#define static
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/gles2.h>
#undef static
}

static size_t next_view_id = 1;

ZenithSurface::ZenithSurface(wlr_surface* surface) : surface{surface}, id{next_view_id++} {
	commit.notify = zenith_surface_commit;
	wl_signal_add(&surface->events.commit, &commit);

	new_subsurface.notify = zenith_subsurface_create;
	wl_signal_add(&surface->events.new_subsurface, &new_subsurface);

	destroy.notify = zenith_surface_destroy;
	wl_signal_add(&surface->events.destroy, &destroy);
}

void zenith_surface_create(wl_listener* listener, void* data) {
	ZenithServer* server = wl_container_of(listener, server, new_surface);
	auto* surface = static_cast<wlr_surface*>(data);
	auto* zenith_surface = new ZenithSurface(surface);
	surface->data = zenith_surface;
	server->surfaces.insert(std::make_pair(zenith_surface->id, zenith_surface));
}

int extract_sync_fd_from_dma_buf(wlr_buffer* buffer, const wlr_dmabuf_attributes& dmabuf_attributes);

int extract_fd_from_native_fence(EGLSyncKHR* sync_out);

void schedule_buffer_commit_on_fd(int fd, size_t view_id, std::shared_ptr<wlr_buffer> buffer);

void zenith_surface_commit(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, commit);
	wlr_surface* surface = zenith_surface->surface;
	wlr_buffer* buffer = &surface->buffer->base;

	SurfaceCommitMessage commit_message{};
	commit_message.view_id = zenith_surface->id;

	wlr_texture* texture = wlr_surface_get_texture(surface);
	if (texture != nullptr && wlr_texture_is_gles2(texture)) {
		wlr_dmabuf_attributes dmabuf_attributes = {};
		bool is_dma = wlr_buffer_get_dmabuf(&surface->buffer->base, &dmabuf_attributes);

		if (is_dma) {
			int sync_fd = extract_sync_fd_from_dma_buf(buffer, dmabuf_attributes);
			if (sync_fd != -1) {
				auto scoped_buffer = scoped_wlr_buffer(buffer, [sync_fd](wlr_buffer* buffer) {
					close(sync_fd);
				});
				schedule_buffer_commit_on_fd(sync_fd, zenith_surface->id, scoped_buffer);
			}
		} else {
			EGLSyncKHR sync = nullptr;
			int fence_fd = extract_fd_from_native_fence(&sync);
			if (fence_fd != -1 && sync != nullptr) {
				wlr_egl* egl = wlr_gles2_renderer_get_egl(ZenithServer::instance()->renderer);
				EGLDisplay egl_display = egl->display;

				auto scoped_buffer = scoped_wlr_buffer(buffer, [fence_fd, egl_display, sync](wlr_buffer* buffer) {
					eglDestroySyncKHR(egl_display, sync);
					close(fence_fd);
				});
				schedule_buffer_commit_on_fd(fence_fd, zenith_surface->id, scoped_buffer);
			}
		}
	}

	SurfaceRole role;
	if (wlr_surface_is_xdg_surface(surface)) {
		role = XDG_SURFACE;
	} else if (wlr_surface_is_subsurface(surface)) {
		role = SUBSURFACE;
	} else {
		role = NONE;
	}

	commit_message.surface = {
		  .role = role,
		  .texture_id = (int) zenith_surface->id,
		  .x = surface->sx,
		  .y = surface->sy,
		  .width = surface->current.width,
		  .height = surface->current.height,
		  .scale = surface->current.scale,
		  .input_region = surface->input_region.extents,
	};

	std::vector<SubsurfaceParentState> below{};
	std::vector<SubsurfaceParentState> above{};

	struct wlr_subsurface* subsurface;
	wl_list_for_each(subsurface, &surface->current.subsurfaces_below, current.link) {
		auto* subsurface_surface = static_cast<ZenithSurface*>(subsurface->surface->data);
		SubsurfaceParentState state = {
			  .id = (int64_t) subsurface_surface->id,
			  .x = subsurface->current.x,
			  .y = subsurface->current.y,
		};
		below.push_back(state);
	}
	wl_list_for_each(subsurface, &surface->current.subsurfaces_above, current.link) {
		auto* subsurface_surface = static_cast<ZenithSurface*>(subsurface->surface->data);
		SubsurfaceParentState state = {
			  .id = (int64_t) subsurface_surface->id,
			  .x = subsurface->current.x,
			  .y = subsurface->current.y,
		};
		above.push_back(state);
	}

	commit_message.subsurfaces_below = std::move(below);
	commit_message.subsurfaces_above = std::move(above);

	if (role == XDG_SURFACE) {
		wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);
		wlr_box visible_bounds = xdg_surface_get_visible_bounds(xdg_surface);
		commit_message.xdg_surface = {
			  .role = xdg_surface->role,
			  .x = visible_bounds.x,
			  .y = visible_bounds.y,
			  .width = visible_bounds.width,
			  .height = visible_bounds.height,
		};

		if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
			wlr_xdg_popup* popup = xdg_surface->popup;
			int64_t parent_id;
			if (popup->parent != nullptr) {
				assert(wlr_surface_is_xdg_surface(popup->parent));
				auto* parent = static_cast<ZenithSurface*>(popup->parent->data);
				parent_id = (int64_t) parent->id;
			} else {
				parent_id = 0;
			}

			const wlr_box& geometry = popup->geometry;
			commit_message.xdg_popup = {
				  .parent_id = parent_id,
				  .x = geometry.x,
				  .y = geometry.y,
				  .width = geometry.width,
				  .height = geometry.height,
			};
		}
	}

	ZenithServer::instance()->embedder_state->commit_surface(commit_message);
}

void zenith_surface_destroy(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, destroy);
	auto* server = ZenithServer::instance();
	server->surface_buffer_chains.erase(zenith_surface->id);
	bool erased = server->surfaces.erase(zenith_surface->id);
	assert(erased);
	// TODO: Send destroy to Flutter.
}

int extract_sync_fd_from_dma_buf(wlr_buffer* buffer, const wlr_dmabuf_attributes& dmabuf_attributes) {
	if (dmabuf_attributes.n_planes <= 0) {
		return -1;
	}
	// All planes should have the same fd.
	int dma_fd = dmabuf_attributes.fd[0];

	// https://gitlab.freedesktop.org/mesa/mesa/-/blob/2e9ce1152e7205116050c5d7da58a2a66d0ed909/src/vulkan/wsi/wsi_common_drm.c#L50
	dma_buf_export_sync_file sync_file = {
		  .flags = DMA_BUF_SYNC_READ,
		  .fd = -1,
	};
	int ret = drmIoctl(dma_fd, DMA_BUF_IOCTL_EXPORT_SYNC_FILE, &sync_file);
	if (ret != 0) {
		return -1;
	}
	int sync_fd = sync_file.fd;
	return sync_fd;
}

// https://registry.khronos.org/EGL/extensions/ANDROID/EGL_ANDROID_native_fence_sync.txt
// Calls `glFence()` and extracts the file descriptor from it, so we can receive a notification
// through the event loop when all GPU commands finish.
int extract_fd_from_native_fence(EGLSyncKHR* sync_out) {
	assert(sync_out != nullptr);

	EGLint attrib_list[] = {
		  EGL_SYNC_NATIVE_FENCE_FD_ANDROID, EGL_NO_NATIVE_FENCE_FD_ANDROID,
		  EGL_NONE,
	};

	wlr_egl* egl = wlr_gles2_renderer_get_egl(ZenithServer::instance()->renderer);
	wlr_egl_make_current(egl);

	EGLSyncKHR sync = eglCreateSyncKHR(egl->display, EGL_SYNC_NATIVE_FENCE_ANDROID, attrib_list);

	auto error = [&] {
		if (sync != EGL_NO_SYNC_KHR) {
			eglDestroySyncKHR(egl->display, sync);
		}
		*sync_out = EGL_NO_SYNC_KHR;
		return -1;
	};

	if (sync == EGL_NO_SYNC_KHR) {
		return error();
	}
	glFlush();

	int fence_fd = eglDupNativeFenceFDANDROID(egl->display, sync);
	if (fence_fd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
		return error();
	}

	*sync_out = sync;
	return fence_fd;
}

// Commits the buffer when the file descriptor becomes readable.
void schedule_buffer_commit_on_fd(int fd, size_t view_id, std::shared_ptr<wlr_buffer> buffer) {
	struct SourceData {
		size_t id;
		std::shared_ptr<wlr_buffer> buffer;
		wl_event_source* source; // I want the source to remove itself.
	};

	auto* source_data = new SourceData{
		  .id = view_id,
		  .buffer = std::move(buffer),
		  .source = nullptr,
	};

	wl_event_loop_fd_func_t func = [](int fd, uint32_t mask, void* data) {
		auto source_data = static_cast<SourceData*>(data);
		auto* server = ZenithServer::instance();
		size_t id = source_data->id;

		std::shared_ptr<SurfaceBufferChain<wlr_buffer>> buffer_chain;
		auto it = server->surface_buffer_chains.find(id);
		if (it == server->surface_buffer_chains.end()) {
			buffer_chain = std::make_shared<SurfaceBufferChain<wlr_buffer>>();
			server->surface_buffer_chains.insert(std::pair(id, buffer_chain));
			server->embedder_state->register_external_texture((int64_t) id);
		} else {
			buffer_chain = it->second;
		}

		buffer_chain->commit_buffer(std::move(source_data->buffer));

		server->embedder_state->mark_external_texture_frame_available((int64_t) id);

		wl_event_source_remove(source_data->source);
		delete source_data;
		return 0;
	};

	wl_event_loop* event_loop = wl_display_get_event_loop(ZenithServer::instance()->display);
	source_data->source = wl_event_loop_add_fd(event_loop, fd, WL_EVENT_READABLE, func, source_data);
}
