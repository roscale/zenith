#pragma once

#include <GLES2/gl2.h>
#include "util/array_view.hpp"
#include "embedder.h"

bool flutter_make_current(void* userdata);

bool flutter_clear_current(void* userdata);

bool flutter_present(void* userdata, const FlutterPresentInfo* present_info);

bool commit_framebuffer(array_view<FlutterRect> view);

uint32_t flutter_fbo_callback(void* userdata);

GLuint attach_framebuffer();

void flutter_vsync_callback(void* userdata, intptr_t baton);

bool flutter_gl_external_texture_frame_callback(void* userdata, int64_t texture_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out);

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata);

bool flutter_make_resource_current(void* userdata);

FlutterTransformation flutter_surface_transformation(void* data);

void flutter_populate_existing_damage(void* user_data, intptr_t fbo_id, FlutterDamage* existing_damage);
