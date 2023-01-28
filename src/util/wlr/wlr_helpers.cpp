#include "wlr_helpers.hpp"
#include <cassert>
#include <cstdio>
#include <GLES2/gl2ext.h>
#include <libdrm/drm_fourcc.h>

extern "C" {
#define static
#include <wlr/util/log.h>
#include <wlr/render/drm_format_set.h>
#include <wlr/render/egl.h>
#include <wlr/render/allocator.h>
#include <wlr/render/interface.h>
#undef static
}

struct wlr_drm_format* get_output_format(wlr_output* output) {
	struct wlr_allocator* allocator = output->allocator;
	assert(allocator != nullptr);

	const struct wlr_drm_format_set* display_formats =
		  wlr_output_get_primary_formats(output, allocator->buffer_caps);
	struct wlr_drm_format* format = output_pick_format(output, display_formats,
	                                                   output->render_format);
	if (format == nullptr) {
		wlr_log(WLR_ERROR, "Failed to pick primary buffer format for output '%s'",
		        output->name);
		return nullptr;
	}
	return format;
}

struct wlr_drm_format*
output_pick_format(struct wlr_output* output, const struct wlr_drm_format_set* display_formats, uint32_t fmt) {
	struct wlr_renderer* renderer = output->renderer;
	struct wlr_allocator* allocator = output->allocator;
	assert(renderer != nullptr && allocator != nullptr);

	const struct wlr_drm_format_set* render_formats =
		  wlr_renderer_get_render_formats(renderer);
	if (render_formats == nullptr) {
		wlr_log(WLR_ERROR, "Failed to get render formats");
		return nullptr;
	}

	const struct wlr_drm_format* render_format =
		  wlr_drm_format_set_get(render_formats, fmt);
	if (render_format == nullptr) {
		wlr_log(WLR_DEBUG, "Renderer doesn't support format 0x%" PRIX32, fmt);
		return nullptr;
	}

	struct wlr_drm_format* format = nullptr;
	if (display_formats != nullptr) {
		const struct wlr_drm_format* display_format =
			  wlr_drm_format_set_get(display_formats, fmt);
		if (display_format == nullptr) {
			wlr_log(WLR_DEBUG, "Output doesn't support format 0x%" PRIX32, fmt);
			return nullptr;
		}
		format = wlr_drm_format_intersect(display_format, render_format);
	} else {
		// The output can display any format
		format = wlr_drm_format_dup(render_format);
	}

	if (format == nullptr) {
		wlr_log(WLR_DEBUG, "Failed to intersect display and render "
		                   "modifiers for format 0x%" PRIX32 " on output '%s",
		        fmt, output->name);
		return nullptr;
	}

	return format;
}

const struct wlr_drm_format_set* wlr_renderer_get_render_formats(struct wlr_renderer* r) {
	if (!r->impl->get_render_formats) {
		return nullptr;
	}
	return r->impl->get_render_formats(r);
}

struct wlr_drm_format* wlr_drm_format_intersect(const struct wlr_drm_format* a, const struct wlr_drm_format* b) {
	assert(a->format == b->format);

	size_t format_cap = a->len < b->len ? a->len : b->len;
	size_t format_size = sizeof(struct wlr_drm_format) +
	                     format_cap * sizeof(a->modifiers[0]);
	auto* format = static_cast<wlr_drm_format*>(calloc(1, format_size));
	if (format == nullptr) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return nullptr;
	}
	format->format = a->format;
	format->capacity = format_cap;

	for (size_t i = 0; i < a->len; i++) {
		for (size_t j = 0; j < b->len; j++) {
			if (a->modifiers[i] == b->modifiers[j]) {
				assert(format->len < format->capacity);
				format->modifiers[format->len] = a->modifiers[i];
				format->len++;
				break;
			}
		}
	}

	// If the intersection is empty, then the formats aren't compatible with
	// each other.
	if (format->len == 0) {
		free(format);
		return nullptr;
	}

	return format;
}

struct wlr_drm_format* wlr_drm_format_dup(const struct wlr_drm_format* format) {
	assert(format->len <= format->capacity);
	size_t format_size = sizeof(struct wlr_drm_format) +
	                     format->capacity * sizeof(format->modifiers[0]);
	auto* duped_format = static_cast<wlr_drm_format*>(malloc(format_size));
	if (duped_format == nullptr) {
		return nullptr;
	}
	memcpy(duped_format, format, format_size);
	return duped_format;
}

struct wlr_egl_context {
	EGLDisplay display;
	EGLContext context;
	EGLSurface draw_surface;
	EGLSurface read_surface;
};

struct wlr_gles2_renderer {
	struct wlr_renderer wlr_renderer;

	float projection[9];
	struct wlr_egl* egl;
	int drm_fd;

	const char* exts_str;
	struct {
		bool EXT_read_format_bgra;
		bool KHR_debug;
		bool OES_egl_image_external;
		bool OES_egl_image;
		bool EXT_texture_type_2_10_10_10_REV;
		bool OES_texture_half_float_linear;
	} exts;

	struct {
		PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
		PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR;
		PFNGLDEBUGMESSAGECONTROLKHRPROC glDebugMessageControlKHR;
		PFNGLPOPDEBUGGROUPKHRPROC glPopDebugGroupKHR;
		PFNGLPUSHDEBUGGROUPKHRPROC glPushDebugGroupKHR;
		PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glEGLImageTargetRenderbufferStorageOES;
	} procs;

	struct {
		struct {
			GLuint program;
			GLint proj;
			GLint color;
			GLint pos_attrib;
		} quad;
		struct wlr_gles2_tex_shader tex_rgba;
		struct wlr_gles2_tex_shader tex_rgbx;
		struct wlr_gles2_tex_shader tex_ext;
	} shaders;

	struct wl_list buffers; // wlr_gles2_buffer.link
	struct wl_list textures; // wlr_gles2_texture.link

	struct wlr_gles2_buffer* current_buffer;
	uint32_t viewport_width, viewport_height;
};

void push_gles2_debug_(struct wlr_gles2_renderer* renderer,
                       const char* file, const char* func) {
	if (!renderer->procs.glPushDebugGroupKHR) {
		return;
	}

	int len = snprintf(NULL, 0, "%s:%s", file, func) + 1;
	char str[len];
	snprintf(str, len, "%s:%s", file, func);
	renderer->procs.glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION_KHR, 1, -1, str);
}

#define push_gles2_debug(renderer) push_gles2_debug_(renderer, _WLR_FILENAME, __func__)

void pop_gles2_debug(struct wlr_gles2_renderer* renderer) {
	if (renderer->procs.glPopDebugGroupKHR) {
		renderer->procs.glPopDebugGroupKHR();
	}
}

void wlr_egl_save_context(struct wlr_egl_context* context) {
	context->display = eglGetCurrentDisplay();
	context->context = eglGetCurrentContext();
	context->draw_surface = eglGetCurrentSurface(EGL_DRAW);
	context->read_surface = eglGetCurrentSurface(EGL_READ);
}

bool wlr_egl_restore_context(struct wlr_egl_context* context) {
	// If the saved context is a null-context, we must use the current
	// display instead of the saved display because eglMakeCurrent() can't
	// handle EGL_NO_DISPLAY.
	EGLDisplay display = context->display == EGL_NO_DISPLAY ?
	                     eglGetCurrentDisplay() : context->display;

	// If the current display is also EGL_NO_DISPLAY, we assume that there
	// is currently no context set and no action needs to be taken to unset
	// the context.
	if (display == EGL_NO_DISPLAY) {
		return true;
	}

	return eglMakeCurrent(display, context->draw_surface,
	                      context->read_surface, context->context);
}


static void destroy_buffer(struct wlr_gles2_buffer* buffer) {
	wl_list_remove(&buffer->link);
	wlr_addon_finish(&buffer->addon);

	struct wlr_egl_context prev_ctx;
	wlr_egl_save_context(&prev_ctx);
	wlr_egl_make_current(buffer->renderer->egl);

	push_gles2_debug(buffer->renderer);

	glDeleteFramebuffers(1, &buffer->fbo);
	glDeleteRenderbuffers(1, &buffer->rbo);

	pop_gles2_debug(buffer->renderer);

	wlr_egl_destroy_image(buffer->renderer->egl, buffer->image);

	wlr_egl_restore_context(&prev_ctx);

	free(buffer);
}

static void handle_buffer_destroy(struct wlr_addon* addon) {
	struct wlr_gles2_buffer* buffer =
		  wl_container_of(addon, buffer, addon);
	destroy_buffer(buffer);
}

static const struct wlr_addon_interface buffer_addon_impl = {
	  .name = "wlr_gles2_buffer",
	  .destroy = handle_buffer_destroy,
};

struct wlr_gles2_buffer* create_buffer(struct wlr_gles2_renderer* renderer, struct wlr_buffer* wlr_buffer) {
	wlr_gles2_buffer* buffer = static_cast<wlr_gles2_buffer*>(calloc(1, sizeof(*buffer)));
	if (buffer == nullptr) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return nullptr;
	}
	buffer->buffer = wlr_buffer;
	buffer->renderer = renderer;

	struct wlr_dmabuf_attributes dmabuf = {};
	if (!wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf)) {
		free(buffer);
		return nullptr;
	}

	bool external_only;
	buffer->image = wlr_egl_create_image_from_dmabuf(renderer->egl,
	                                                 &dmabuf, &external_only);
	if (buffer->image == EGL_NO_IMAGE_KHR) {
		free(buffer);
		return nullptr;
	}

	push_gles2_debug(renderer);

	glGenRenderbuffers(1, &buffer->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer->rbo);
	renderer->procs.glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER,
	                                                       buffer->image);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &buffer->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
	                          GL_RENDERBUFFER, buffer->rbo);
	GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	pop_gles2_debug(renderer);

	if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
		wlr_log(WLR_ERROR, "Failed to create FBO");
		goto error_image;
	}

	wlr_addon_init(&buffer->addon, &wlr_buffer->addons, renderer,
	               &buffer_addon_impl);

	wl_list_insert(&renderer->buffers, &buffer->link);

	wlr_log(WLR_DEBUG, "Created GL FBO for buffer %dx%d",
	        wlr_buffer->width, wlr_buffer->height);

	return buffer;

	error_image:
	wlr_egl_destroy_image(renderer->egl, buffer->image);
	free(buffer);
	return nullptr;
}

EGLImageKHR
wlr_egl_create_image_from_dmabuf(struct wlr_egl* egl, struct wlr_dmabuf_attributes* attributes, bool* external_only) {
	if (!egl->exts.KHR_image_base || !egl->exts.EXT_image_dma_buf_import) {
		wlr_log(WLR_ERROR, "dmabuf import extension not present");
		return NULL;
	}

	if (attributes->modifier != DRM_FORMAT_MOD_INVALID &&
	    attributes->modifier != DRM_FORMAT_MOD_LINEAR &&
	    !egl->has_modifiers) {
		wlr_log(WLR_ERROR, "EGL implementation doesn't support modifiers");
		return NULL;
	}

	unsigned int atti = 0;
	EGLint attribs[50];
	attribs[atti++] = EGL_WIDTH;
	attribs[atti++] = attributes->width;
	attribs[atti++] = EGL_HEIGHT;
	attribs[atti++] = attributes->height;
	attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
	attribs[atti++] = attributes->format;

	struct {
		EGLint fd;
		EGLint offset;
		EGLint pitch;
		EGLint mod_lo;
		EGLint mod_hi;
	} attr_names[WLR_DMABUF_MAX_PLANES] = {
		  {
				EGL_DMA_BUF_PLANE0_FD_EXT,
				EGL_DMA_BUF_PLANE0_OFFSET_EXT,
				EGL_DMA_BUF_PLANE0_PITCH_EXT,
				EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT,
				EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT
		  },
		  {
				EGL_DMA_BUF_PLANE1_FD_EXT,
				EGL_DMA_BUF_PLANE1_OFFSET_EXT,
				EGL_DMA_BUF_PLANE1_PITCH_EXT,
				EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT,
				EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT
		  },
		  {
				EGL_DMA_BUF_PLANE2_FD_EXT,
				EGL_DMA_BUF_PLANE2_OFFSET_EXT,
				EGL_DMA_BUF_PLANE2_PITCH_EXT,
				EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT,
				EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT
		  },
		  {
				EGL_DMA_BUF_PLANE3_FD_EXT,
				EGL_DMA_BUF_PLANE3_OFFSET_EXT,
				EGL_DMA_BUF_PLANE3_PITCH_EXT,
				EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT,
				EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT
		  }
	};

	for (int i = 0; i < attributes->n_planes; i++) {
		attribs[atti++] = attr_names[i].fd;
		attribs[atti++] = attributes->fd[i];
		attribs[atti++] = attr_names[i].offset;
		attribs[atti++] = attributes->offset[i];
		attribs[atti++] = attr_names[i].pitch;
		attribs[atti++] = attributes->stride[i];
		if (egl->has_modifiers &&
		    attributes->modifier != DRM_FORMAT_MOD_INVALID) {
			attribs[atti++] = attr_names[i].mod_lo;
			attribs[atti++] = attributes->modifier & 0xFFFFFFFF;
			attribs[atti++] = attr_names[i].mod_hi;
			attribs[atti++] = attributes->modifier >> 32;
		}
	}

	// Our clients don't expect our usage to trash the buffer contents
	attribs[atti++] = EGL_IMAGE_PRESERVED_KHR;
	attribs[atti++] = EGL_TRUE;

	attribs[atti++] = EGL_NONE;
	assert(atti < sizeof(attribs) / sizeof(attribs[0]));

	EGLImageKHR image = egl->procs.eglCreateImageKHR(egl->display, EGL_NO_CONTEXT,
	                                                 EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
	if (image == EGL_NO_IMAGE_KHR) {
		wlr_log(WLR_ERROR, "eglCreateImageKHR failed");
		return EGL_NO_IMAGE_KHR;
	}

	*external_only = !wlr_drm_format_set_has(&egl->dmabuf_render_formats,
	                                         attributes->format, attributes->modifier);
	return image;
}

bool wlr_egl_destroy_image(struct wlr_egl* egl, EGLImage image) {
	if (!egl->exts.KHR_image_base) {
		return false;
	}
	if (!image) {
		return true;
	}
	return egl->procs.eglDestroyImageKHR(egl->display, image);
}
