#pragma once

#include <cstddef>

extern "C" {
#include <wlr/types/wlr_xdg_shell.h>
#include <epoxy/gl.h>
}

struct ZenithServer;

struct ZenithView {
	ZenithView(ZenithServer* server);

	ZenithServer* server;
	size_t id;
	bool mapped = false;

	/*
	 * Activate the view and make the keyboard enter the surface of this view.
	 */
	virtual void focus() = 0;

	virtual void close() = 0;

	virtual void pointer_hover(double x, double y) = 0;

	virtual void resize(uint32_t width, uint32_t height) = 0;

	virtual void render_to_fbo(GLuint fbo) = 0;

	virtual ~ZenithView() = default;
};

template<class T>
struct render_data {
	T* view;
	GLuint view_fbo;
};
