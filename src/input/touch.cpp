#include "server.hpp"
#include "touch.hpp"
#include "time.hpp"
#include "embedder.h"

ZenithTouchDevice::ZenithTouchDevice(ZenithServer* server, wlr_input_device* wlr_device)
	  : server(server), wlr_device(wlr_device) {
	touch_down.notify = touch_down_handle;
	wl_signal_add(&wlr_device->touch->events.down, &touch_down);

	touch_motion.notify = touch_motion_handle;
	wl_signal_add(&wlr_device->touch->events.motion, &touch_motion);

	touch_up.notify = touch_up_handle;
	wl_signal_add(&wlr_device->touch->events.up, &touch_up);

	touch_cancel.notify = touch_cancel_handle;
	wl_signal_add(&wlr_device->touch->events.cancel, &touch_cancel);
}

void touch_down_handle(wl_listener* listener, void* data) {
	ZenithTouchDevice* touch_device = wl_container_of(listener, touch_device, touch_down);
	ZenithServer* server = touch_device->server;
	auto* event = static_cast<wlr_event_touch_down*>(data);

	FlutterPointerEvent e = {};
	e.struct_size = sizeof(FlutterPointerEvent);
	e.phase = kDown;
	e.timestamp = current_time_microseconds();
	// Map from [0, 1] to [screen_width, screen_height].
	e.x = event->x * server->output->wlr_output->width;
	e.y = event->y * server->output->wlr_output->height;

	touch_device->last_touch_coordinates[event->touch_id] = std::pair(event->x, event->y);

	e.device_kind = kFlutterPointerDeviceKindTouch;
	e.signal_kind = kFlutterPointerSignalKindNone;
	e.device = event->touch_id;

	FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
}

void touch_motion_handle(wl_listener* listener, void* data) {
	ZenithTouchDevice* touch_device = wl_container_of(listener, touch_device, touch_motion);
	ZenithServer* server = touch_device->server;
	auto* event = static_cast<wlr_event_touch_motion*>(data);

	FlutterPointerEvent e = {};
	e.struct_size = sizeof(FlutterPointerEvent);
	e.phase = kMove;

	e.timestamp = current_time_microseconds();
	// Map from [0, 1] to [screen_width, screen_height].
	e.x = event->x * server->output->wlr_output->width;
	e.y = event->y * server->output->wlr_output->height;

	touch_device->last_touch_coordinates[event->touch_id] = std::pair(event->x, event->y);

	e.device_kind = kFlutterPointerDeviceKindTouch;
	e.signal_kind = kFlutterPointerSignalKindNone;
	e.device = event->touch_id;

	FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
}

void touch_up_handle(wl_listener* listener, void* data) {
	ZenithTouchDevice* touch_device = wl_container_of(listener, touch_device, touch_up);
	ZenithServer* server = touch_device->server;
	auto* event = static_cast<wlr_event_touch_up*>(data);

	FlutterPointerEvent e = {};
	e.struct_size = sizeof(FlutterPointerEvent);
	e.phase = kUp;

	auto last_coordinates = touch_device->last_touch_coordinates[event->touch_id];
	// Map from [0, 1] to [screen_width, screen_height].
	e.x = last_coordinates.first * server->output->wlr_output->width;
	e.y = last_coordinates.second * server->output->wlr_output->height;

	e.timestamp = current_time_microseconds();
	e.device_kind = kFlutterPointerDeviceKindTouch;
	e.signal_kind = kFlutterPointerSignalKindNone;
	e.device = event->touch_id;

	FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
}

void touch_cancel_handle(wl_listener* listener, void* data) {
	ZenithTouchDevice* touch_device = wl_container_of(listener, touch_device, touch_cancel);
	ZenithServer* server = touch_device->server;
	auto* event = static_cast<wlr_event_touch_cancel*>(data);

	FlutterPointerEvent e = {};
	e.struct_size = sizeof(FlutterPointerEvent);
	e.phase = kCancel;

	auto last_coordinates = touch_device->last_touch_coordinates[event->touch_id];
	e.x = last_coordinates.first * server->output->wlr_output->width;
	e.y = last_coordinates.second * server->output->wlr_output->height;

	e.timestamp = current_time_microseconds();
	// Map from [0, 1] to [screen_width, screen_height].
	e.device_kind = kFlutterPointerDeviceKindTouch;
	e.signal_kind = kFlutterPointerSignalKindNone;
	e.device = event->touch_id;

	FlutterEngineSendPointerEvent(server->flutter_engine_state->engine, &e, 1);
}