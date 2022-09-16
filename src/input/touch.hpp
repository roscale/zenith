#pragma once

extern "C" {
#define static
#include "wlr/types/wlr_input_device.h"
#include "wlr/types/wlr_touch.h"
#undef static
}

struct ZenithServer;

struct ZenithTouchDevice {
	ZenithTouchDevice(ZenithServer* server, wlr_input_device* wlr_device);

	ZenithServer* server;
	wlr_input_device* wlr_device;

	wl_listener touch_down{};
	wl_listener touch_motion{};
	wl_listener touch_up{};
	wl_listener touch_cancel{};

	std::unordered_map<int32_t, std::pair<double, double>> last_touch_coordinates;
};

void touch_down_handle(wl_listener* listener, void* data);

void touch_motion_handle(wl_listener* listener, void* data);

void touch_up_handle(wl_listener* listener, void* data);

void touch_cancel_handle(wl_listener* listener, void* data);
