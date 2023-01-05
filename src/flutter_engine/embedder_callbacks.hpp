#pragma once

#include "embedder.h"

bool flutter_make_current(void* userdata);

bool flutter_clear_current(void* userdata);

bool flutter_present(void* userdata);

uint32_t flutter_fbo_with_frame_info_callback(void* userdata, const FlutterFrameInfo* frame_info);

void flutter_vsync_callback(void* userdata, intptr_t baton);

bool flutter_gl_external_texture_frame_callback(void* userdata, int64_t texture_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out);

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata);

bool flutter_make_resource_current(void* userdata);

int flutter_execute_expired_tasks_timer(void* data);

FlutterTransformation flutter_surface_transformation(void* data);
