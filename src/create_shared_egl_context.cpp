#include "create_shared_egl_context.hpp"

#include <cstring>
#include <iostream>

extern "C" {
//#include <EGL/egl.h>
//#define static
#include <wlr/render/egl.h>
//#undef static
}

static const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE,
};

static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
};

struct wlr_egl* create_shared_egl_context(struct wlr_egl* egl) {
	struct wlr_egl* shared_egl;

	EGLConfig egl_config;
	EGLint matched = 0;
	if (!eglChooseConfig(egl->display, config_attribs, &egl_config, 1, &matched)) {
		std::cerr << "eglChooseConfig failed" << std::endl;
		return nullptr;
	}
	if (matched == 0) {
		std::cerr << "Failed to match an EGL config" << std::endl;
		return nullptr;
	}

	EGLContext shared_egl_context = eglCreateContext(egl->display, egl_config, egl->context, context_attribs);
	if (shared_egl_context == EGL_NO_CONTEXT) {
		std::cerr << "Failed to create EGL context" << std::endl;
		return nullptr;
	}

	shared_egl = new(std::nothrow) wlr_egl();
	if (shared_egl == nullptr) {
		return nullptr;
	}

	memcpy(shared_egl, egl, sizeof(struct wlr_egl));
	shared_egl->context = shared_egl_context;
	return shared_egl;
}