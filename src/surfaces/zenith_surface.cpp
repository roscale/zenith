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
static size_t next_texture_id = 1;

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

void commit_surface(SurfaceCommitMessage* commit_message);

void schedule_buffer_commit_on_fd(int fd, std::unique_ptr<SurfaceCommitMessage> commit_message);

void zenith_surface_commit(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, commit);
	wlr_surface* surface = zenith_surface->surface;
	wlr_buffer* buffer = &surface->buffer->base;
	auto* server = ZenithServer::instance();

	auto commit_message = std::make_unique<SurfaceCommitMessage>();
	commit_message->view_id = zenith_surface->id;

	SurfaceRole role;
	if (wlr_surface_is_xdg_surface(surface) and wlr_xdg_surface_from_wlr_surface(surface) != nullptr) {
		// TODO: maybe report a bug in wlroots
		// This != nullptr check is necessary because when I close the easyeffects window, the compositor crashes.
		// Apparently, a surface can have the xdg role but have a null xdg surface, which I didn't expect.
		role = SurfaceRole::XDG_SURFACE;
	} else if (wlr_surface_is_subsurface(surface) and wlr_subsurface_from_wlr_surface(surface) != nullptr) {
		role = SurfaceRole::SUBSURFACE;
	} else {
		role = SurfaceRole::NONE;
	}

	size_t texture_id;
	auto it = server->texture_ids_per_surface_id.find(zenith_surface->id);
	if (it != server->texture_ids_per_surface_id.end() &&
	    zenith_surface->old_buffer_width == surface->current.buffer_width &&
	    zenith_surface->old_buffer_height == surface->current.buffer_height) {
		assert(!it->second.empty());
		texture_id = it->second.back();
	} else {
		texture_id = next_texture_id++;
		server->surface_id_per_texture_id[texture_id] = zenith_surface->id;
		server->texture_ids_per_surface_id[zenith_surface->id].push_back(texture_id);
	}

	zenith_surface->old_buffer_width = surface->current.buffer_width;
	zenith_surface->old_buffer_height = surface->current.buffer_height;

	commit_message->surface = {
		  .role = role,
		  .texture_id = (int) texture_id,
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

	commit_message->subsurfaces_below = std::move(below);
	commit_message->subsurfaces_above = std::move(above);

	if (role == SurfaceRole::XDG_SURFACE) {
		wlr_xdg_surface* xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);
		wlr_box visible_bounds = xdg_surface_get_visible_bounds(xdg_surface);
		commit_message->xdg_surface = {
			  // Wlroots actually provides a map signal to which I could register a callback,
			  // but it has been unreliable in my experience.
			  //
			  // Some Firefox popups weren't visible because I never received a map event
			  // for those surfaces even though they were mapped.
			  //
			  // In addition, this helps avoid inconsistent state on the Flutter side
			  // because this mapped property is sent atomically with the other surface state.
			  //
			  // If the mapped property were to be sent as a different message after sending the
			  // other state, I think Flutter could choose to deserialize the first message before
			  // rendering the first frame, and the second message before rendering the next frame,
			  // so it would be rendering based on an inconsistent state.
			  .mapped = xdg_surface->mapped,
			  .role = xdg_surface->role,
			  .x = visible_bounds.x,
			  .y = visible_bounds.y,
			  .width = visible_bounds.width,
			  .height = visible_bounds.height,
		};
		switch (xdg_surface->role) {
			case WLR_XDG_SURFACE_ROLE_TOPLEVEL: {
				auto it = server->toplevel_decorations.find(zenith_surface->id);
				if (it != server->toplevel_decorations.end()) {
					commit_message->toplevel_decoration = (ToplevelDecoration) it->second->wlr_toplevel_decoration->pending.mode;
				}
				const char* title = xdg_surface->toplevel->title;
				if (title != nullptr) {
					commit_message->toplevel_title = title;
				}
				const char* app_id = xdg_surface->toplevel->app_id;
				if (app_id != nullptr) {
					commit_message->toplevel_app_id = app_id;
				}
				break;
			}
			case WLR_XDG_SURFACE_ROLE_POPUP: {
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
				commit_message->xdg_popup = {
					  .parent_id = parent_id,
					  .x = geometry.x,
					  .y = geometry.y,
					  .width = geometry.width,
					  .height = geometry.height,
				};
				break;
			}
			case WLR_XDG_SURFACE_ROLE_NONE:
				break;
		}
	}

	std::shared_ptr<wlr_buffer> scoped_buffer = nullptr;

	// TODO: Revisit the idea of explicit synchronization.
	// And I need to implement canceling of scheduled commit if the texture of the next one is already ready.

	int wait_for_fd = -1;

	wlr_texture* texture = wlr_surface_get_texture(surface);
	if (texture != nullptr && wlr_texture_is_gles2(texture)) {
		wlr_dmabuf_attributes dmabuf_attributes = {};
		bool is_dma = wlr_buffer_get_dmabuf(&surface->buffer->base, &dmabuf_attributes);

		if (is_dma) {
			int sync_fd = extract_sync_fd_from_dma_buf(buffer, dmabuf_attributes);
			if (sync_fd != -1) {
				scoped_buffer = scoped_wlr_buffer(buffer, [sync_fd](wlr_buffer* buffer) {
					close(sync_fd);
				});
				wait_for_fd = sync_fd;
			} else {
				scoped_buffer = scoped_wlr_buffer(buffer);
			}
		} else {
			EGLSyncKHR sync = nullptr;
			int fence_fd = extract_fd_from_native_fence(&sync);
			if (fence_fd != -1 && sync != nullptr) {
				wlr_egl* egl = wlr_gles2_renderer_get_egl(ZenithServer::instance()->renderer);
				EGLDisplay egl_display = egl->display;
				scoped_buffer = scoped_wlr_buffer(buffer, [fence_fd, egl_display, sync](wlr_buffer* buffer) {
					eglDestroySyncKHR(egl_display, sync);
					close(fence_fd);
				});
				wait_for_fd = fence_fd;
			} else {
				scoped_buffer = scoped_wlr_buffer(buffer);
			}
		}
	}

	commit_message->buffer = scoped_buffer;

	(void) wait_for_fd;
//	if (wait_for_fd != -1) {
//		schedule_buffer_commit_on_fd(wait_for_fd, std::move(commit_message));
//	} else {
		commit_surface(commit_message.get());
//	}
}

void zenith_surface_destroy(wl_listener* listener, void* data) {
	ZenithSurface* zenith_surface = wl_container_of(listener, zenith_surface, destroy);
	auto* server = ZenithServer::instance();
//	server->surface_buffer_chains.erase(zenith_surface->id);
	size_t id = zenith_surface->id;
	bool erased = server->surfaces.erase(zenith_surface->id);
	assert(erased);

	server->embedder_state->destroy_surface(id);
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

void commit_surface(SurfaceCommitMessage* commit_message) {
	auto* server = ZenithServer::instance();
	if (server->surfaces.count(commit_message->view_id) == 0) {
		// This function could be called later when the GPU finished rendering surface's texture.
		// At that point in time, the surface could be destroyed.
		return;
	}
	assert(server->surfaces.count(commit_message->view_id) == 1);

	size_t texture_id = commit_message->surface.texture_id;

	std::shared_ptr<SurfaceBufferChain<wlr_buffer>> buffer_chain;
	auto it = server->surface_buffer_chains.find(texture_id);
	if (it == server->surface_buffer_chains.end()) {
		buffer_chain = std::make_shared<SurfaceBufferChain<wlr_buffer>>();
		server->surface_buffer_chains.insert(std::pair(texture_id, buffer_chain));
		server->embedder_state->register_external_texture((int64_t) texture_id);
	} else {
		buffer_chain = it->second;
	}

	buffer_chain->commit_buffer(std::move(commit_message->buffer));
	server->embedder_state->mark_external_texture_frame_available((int64_t) texture_id);
	server->embedder_state->commit_surface(*commit_message);
}

// Commits the buffer when the file descriptor becomes readable.
void schedule_buffer_commit_on_fd(int fd, std::unique_ptr<SurfaceCommitMessage> commit_message) {
	struct SourceData {
		std::unique_ptr<SurfaceCommitMessage> commit_message;
		wl_event_source* source; // I want the source to remove itself.
	};

	auto* source_data = new SourceData{
		  .commit_message = std::move(commit_message),
		  .source = nullptr,
	};

	wl_event_loop_fd_func_t func = [](int fd, uint32_t mask, void* data) {
		auto source_data = static_cast<SourceData*>(data);

		commit_surface(source_data->commit_message.get());

		wl_event_source_remove(source_data->source);
		delete source_data;
		return 0;
	};

	wl_event_loop* event_loop = wl_display_get_event_loop(ZenithServer::instance()->display);
	source_data->source = wl_event_loop_add_fd(event_loop, fd, WL_EVENT_READABLE, func, source_data);
}
