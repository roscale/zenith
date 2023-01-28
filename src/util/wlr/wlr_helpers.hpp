#pragma once

// TODO: Re-extract the functions when updating wlroots.

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <wayland-util.h>

extern "C" {
#include <wlr/types/wlr_output.h>
#define static
#include <wlr/util/addon.h>
#undef static
}

struct wlr_drm_format* get_output_format(wlr_output* output);

struct wlr_drm_format* output_pick_format(struct wlr_output* output,
                                          const struct wlr_drm_format_set* display_formats,
                                          uint32_t fmt);

const struct wlr_drm_format_set* wlr_renderer_get_render_formats(
	  struct wlr_renderer* r);

struct wlr_drm_format* wlr_drm_format_intersect(
	  const struct wlr_drm_format* a, const struct wlr_drm_format* b);

struct wlr_drm_format* wlr_drm_format_dup(const struct wlr_drm_format* format);

struct wlr_gles2_buffer* create_buffer(struct wlr_gles2_renderer* renderer,
                                       struct wlr_buffer* wlr_buffer);

struct wlr_gles2_buffer {
	struct wlr_buffer* buffer;
	struct wlr_gles2_renderer* renderer;
	struct wl_list link; // wlr_gles2_renderer.buffers

	EGLImageKHR image;
	GLuint rbo;
	GLuint fbo;

	struct wlr_addon addon;
};

struct wlr_gles2_tex_shader {
	GLuint program;
	GLint proj;
	GLint tex;
	GLint alpha;
	GLint pos_attrib;
	GLint tex_attrib;
};

EGLImageKHR wlr_egl_create_image_from_dmabuf(struct wlr_egl* egl,
                                             struct wlr_dmabuf_attributes* attributes, bool* external_only);

bool wlr_egl_destroy_image(struct wlr_egl* egl, EGLImage image);
