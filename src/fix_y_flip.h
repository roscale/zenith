#pragma once

#include <GLES2/gl2.h>

struct fix_y_flip_state {
	GLuint program;
	GLuint vbo;
	GLuint ebo;
	GLuint offscreen_framebuffer;
	GLuint framebuffer_texture;
};

struct fix_y_flip_state fix_y_flip_init_state(int width, int height);

void bind_offscreen_framebuffer(struct fix_y_flip_state* state);

void render_to_fbo(struct fix_y_flip_state* state, unsigned int fbo);