#include "fix_y_flip.hpp"
#include <cstdlib>
#include <iostream>
#include <GL/gl.h>

static const char* vertexShaderSource = "attribute vec3 position;\n"
                                        "attribute vec2 texcoord;\n"
                                        "varying vec2 vtexcoord;\n"
                                        "void main()\n"
                                        "{\n"
                                        "   gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
                                        "   vtexcoord = texcoord;\n"
                                        "}\0";

static const char* fragmentShaderSource = "precision mediump float;\n"
                                          "uniform sampler2D tex;\n"
                                          "varying vec2 vtexcoord;\n"
                                          "void main()\n"
                                          "{\n"
                                          "   gl_FragColor = texture2D(tex, vtexcoord);\n"
                                          "}\n\0";

static const float vertices[] = {
	  // pos_x, pos_y, pos_z, tex_x, tex_y
	  // Last column has the y-flip fix.
	  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // top right
	  1.0f, -1.0f, 0.0f, 1.0f, 1.0f,   // bottom right
	  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,   // bottom left
	  -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,    // top left
};
static const unsigned int indices[] = {
	  0, 1, 3,
	  1, 2, 3,
};

struct fix_y_flip_state fix_y_flip_init_state(int width, int height) {
	int success;
	char infoLog[512];

	// Create vertex shader.
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cerr << infoLog;
		exit(1);
	}

	// Create fragment shader.
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cerr << infoLog;
		exit(1);
	}

	// Link the program.
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texcoord");

	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cerr << infoLog;
		exit(1);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Upload the quad on the GPU.
	unsigned int vbo, ebo;

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Create the intermediate framebuffer.
	GLuint offscreen_framebuffer;
	glGenFramebuffers(1, &offscreen_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer);

	// Create a texture and attach it to the framebuffer.
	GLuint framebuffer_texture;
	glGenTextures(1, &framebuffer_texture);

	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);

	// Create the depth/stencil renderbuffer and attach it to the framebuffer.
	GLuint depth_stencil_renderbuffer;
	glGenRenderbuffers(1, &depth_stencil_renderbuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_renderbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_renderbuffer);

	// Abort if the framebuffer was not correctly created.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Incomplete framebuffer: " << status << std::endl;
		exit(1);
	}

	fix_y_flip_state state = {
		  .program = shaderProgram,
		  .vbo = vbo,
		  .ebo = ebo,
		  .offscreen_framebuffer = offscreen_framebuffer,
		  .framebuffer_texture = framebuffer_texture,
		  .depth_stencil_renderbuffer = depth_stencil_renderbuffer,
	};
	return state;
}

void bind_offscreen_framebuffer(fix_y_flip_state* state) {
	glBindFramebuffer(GL_FRAMEBUFFER, state->offscreen_framebuffer);
}

void render_to_fbo(fix_y_flip_state* state, unsigned int fbo) {
	// Backup context state.
	GLint framebuffer_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_binding);
	GLint vbo_binding;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo_binding);
	GLint ebo_binding;
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo_binding);
	GLint current_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
	GLint active_texture;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
	GLint texture_binding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);

	glUseProgram(state->program);

	GLint tex = glGetUniformLocation(state->program, "tex");
	glUniform1i(tex, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->framebuffer_texture);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Restore context state.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_binding);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_binding);
	glUseProgram(current_program);
	glActiveTexture(active_texture);
	glBindTexture(GL_TEXTURE_2D, texture_binding);
}