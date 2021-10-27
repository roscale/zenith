#pragma once

#include "flutland_structs.h"
#include "embedder.h"

#include <stdbool.h>
#include <stdint.h>

FlutterEngine run_flutter(struct flutland_output* output);

bool flutter_make_current(void* userdata);

bool flutter_clear_current(void* userdata);

bool flutter_present(void* userdata);

uint32_t flutter_fbo_callback(void* userdata);

void vsync_callback(void* userdata, intptr_t baton);

void start_rendering(void* userdata);