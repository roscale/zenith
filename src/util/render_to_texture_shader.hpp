#pragma once

#include <epoxy/gl.h>

struct RenderToTextureShader {
	GLuint program = 0;
	GLuint vbo = 0;
	GLuint y_flipped_vbo = 0;
	GLuint ebo = 0;

	static RenderToTextureShader* instance();

	void
	render(GLuint texture, int x, int y, size_t width, size_t height, GLuint framebuffer, bool flip_y_axis = false);

private:
	static RenderToTextureShader* _instance;

	RenderToTextureShader();
};