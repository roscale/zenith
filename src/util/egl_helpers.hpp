#pragma once

#include <EGL/egl.h>

/// wlroots 0.15 took away some APIs I actually used, so here is some code I copied over.

struct ZenithEglContext {
	EGLDisplay display;
	EGLContext context;
	EGLSurface draw_surface;
	EGLSurface read_surface;
};

void zenith_egl_save_context(struct ZenithEglContext* context);

bool zenith_egl_restore_context(struct ZenithEglContext* context);