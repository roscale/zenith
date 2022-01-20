#pragma once

/*
 * OpenGL rendering is done upside down for some reason. To fix this, we render to a texture
 * attached to an intermediate framebuffer and then we blit this texture to a quad on the final
 * framebuffer in the right orientation.
 */

//#include <GLES2/gl2.h>
#include <epoxy/gl.h>

struct fix_y_flip_state {
	GLuint program;
	GLuint vbo;
	GLuint ebo;
	GLuint offscreen_framebuffer;
	GLuint framebuffer_texture;
	GLuint depth_stencil_renderbuffer;
};

fix_y_flip_state fix_y_flip_init_state(int width, int height);

void bind_offscreen_framebuffer(fix_y_flip_state* state);

void render_to_fbo(fix_y_flip_state* state, unsigned int fbo);