#pragma once

struct ZenithView;

struct RenderData {
	ZenithView* view;
	GLuint view_fbo;
};

void render_view_to_framebuffer(ZenithView* view, GLuint view_fbo);