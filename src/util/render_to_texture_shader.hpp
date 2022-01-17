#pragma once

#include <GLES2/gl2.h>

struct RenderToTextureShader {
	GLuint program = 0;
	GLuint vbo = 0;
	GLuint ebo = 0;

	static RenderToTextureShader* instance();

	void render(GLuint texture, int x, int y, size_t width, size_t height, GLuint framebuffer);

private:
	static RenderToTextureShader* _instance;

	RenderToTextureShader();
};