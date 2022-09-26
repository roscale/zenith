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
	  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
	  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom right
	  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,   // bottom left
	  -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,    // top left
};

static const float y_flipped_vertices[] = {
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
	glGenBuffers(1, &y_flipped_vbo);
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, y_flipped_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(y_flipped_vertices), y_flipped_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Restore context state.
	glBindBuffer(GL_ARRAY_BUFFER, vbo_binding);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_binding);
}

void RenderToTextureShader::render(GLuint texture, int x, int y, size_t width, size_t height, GLuint framebuffer,
                                   bool flip_y_axis) {
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

	GLint vertex_attrib_0_enabled;
	glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertex_attrib_0_enabled);
	GLint vertex_attrib_0_size;
	glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertex_attrib_0_size);
	GLint vertex_attrib_0_type;
	glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertex_attrib_0_type);
	GLint vertex_attrib_0_normalized;
	glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertex_attrib_0_normalized);
	GLint vertex_attrib_0_stride;
	glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertex_attrib_0_stride);
	void* vertex_attrib_0_pointer;
	glGetVertexAttribPointerv(0, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertex_attrib_0_pointer);

	GLint vertex_attrib_1_enabled;
	glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertex_attrib_1_enabled);
	GLint vertex_attrib_1_size;
	glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertex_attrib_1_size);
	GLint vertex_attrib_1_type;
	glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertex_attrib_1_type);
	GLint vertex_attrib_1_normalized;
	glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertex_attrib_1_normalized);
	GLint vertex_attrib_1_stride;
	glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertex_attrib_1_stride);
	void* vertex_attrib_1_pointer;
	glGetVertexAttribPointerv(1, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertex_attrib_1_pointer);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glBindBuffer(GL_ARRAY_BUFFER, flip_y_axis ? y_flipped_vbo : vbo);

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

	glViewport(x, y, (int) width, (int) height);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Restore context state.
	glUseProgram(current_program);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_binding);
	glBindTexture(GL_TEXTURE_2D, texture_binding);
	glActiveTexture(active_texture);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_binding);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_binding);

	glVertexAttribPointer(0, vertex_attrib_0_size, vertex_attrib_0_type, vertex_attrib_0_normalized,
	                      vertex_attrib_0_stride, vertex_attrib_0_pointer);
	if (vertex_attrib_0_enabled) {
		glEnableVertexAttribArray(0);
	} else {
		glDisableVertexAttribArray(0);
	}

	glVertexAttribPointer(1, vertex_attrib_1_size, vertex_attrib_1_type, vertex_attrib_1_normalized,
	                      vertex_attrib_1_stride, vertex_attrib_1_pointer);
	if (vertex_attrib_1_enabled) {
		glEnableVertexAttribArray(1);
	} else {
		glDisableVertexAttribArray(1);
	}
}

RenderToTextureShader* RenderToTextureShader::_instance = nullptr;

RenderToTextureShader* RenderToTextureShader::instance() {
	if (_instance == nullptr) {
		_instance = new RenderToTextureShader();
	}
	return _instance;
}
