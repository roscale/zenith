#pragma once

#include "embedder.h"

bool flutter_make_current(void* userdata);

bool flutter_clear_current(void* userdata);

bool flutter_present(void* userdata);

uint32_t flutter_fbo_callback(void* userdata);

void flutter_vsync_callback(void* userdata, intptr_t baton);

bool flutter_gl_external_texture_frame_callback(void* userdata, int64_t view_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out);

void flutter_execute_platform_tasks(void* data);

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata);

bool flutter_make_resource_current(void* userdata);