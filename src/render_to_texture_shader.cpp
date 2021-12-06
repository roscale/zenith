#include <iostream>
#include "render_to_texture_shader.hpp"

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
	  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
	  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom right
	  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,   // bottom left
	  -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,    // top left
};
static const unsigned int indices[] = {
	  0, 1, 3,
	  1, 2, 3,
};

RenderToTextureShader::RenderToTextureShader() {
	// Backup context state.
	GLint vbo_binding;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo_binding);
	GLint ebo_binding;
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo_binding);

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
	program = shaderProgram;
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
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Restore context state.
	glBindBuffer(GL_ARRAY_BUFFER, vbo_binding);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_binding);
}

void RenderToTextureShader::render(GLuint texture, size_t width, size_t height, GLuint framebuffer) {
	// Backup context state.
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
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

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);

	// texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glUseProgram(program);

	GLint tex = glGetUniformLocation(program, "tex");
	glUniform1i(tex, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Required before rendering a texture.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glViewport(0, 0, width, height);

	// Clear with transparency.
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Restore context state.
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_binding);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_binding);
	glUseProgram(current_program);
	glActiveTexture(active_texture);
	glBindTexture(GL_TEXTURE_2D, texture_binding);
}
