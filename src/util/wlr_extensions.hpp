#pragma once

/**
 * Notify the seat that this is a global gesture and the client should cancel
 * processing it. The event will go to the client for the surface given.
 * This function does not respect touch grabs: you probably want
 * `wlr_seat_touch_notify_cancel()` instead.
 * TODO: remove this function when updating wlroots
 */
void wlr_seat_touch_send_cancel(struct wlr_seat* seat, struct wlr_surface* surface);

/**
 * Notify the seat that this is a global gesture and the client should
 * cancel processing it. Defers to any grab of the touch device.
 * TODO: remove this function when updating wlroots
 */
void wlr_seat_touch_notify_cancel(struct wlr_seat* seat, struct wlr_surface* surface);
