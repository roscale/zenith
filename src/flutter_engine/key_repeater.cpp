#include "server.hpp"
#include "key_repeater.hpp"

void KeyRepeater::schedule_repeat(KeyboardKeyEventMessage repeated_event) {
	cancel_repeat();

	auto* server = ZenithServer::instance();

	channel<int32_t> delay_chan = {};
	channel<int32_t> rate_chan = {};
	server->callable_queue.enqueue([server, &delay_chan, &rate_chan] {
		wlr_keyboard* keyboard = server->seat->keyboard_state.keyboard;
		if (keyboard != nullptr) {
			delay_chan.write(keyboard->repeat_info.delay);
			rate_chan.write(keyboard->repeat_info.rate);
		}
	});
	delay = delay_chan.read();
	rate = rate_chan.read();

	server->embedder_state->callable_queue.enqueue([this, server, repeated_event] {
		this->repeated_event = repeated_event;
		this->repeated_event.state = KeyboardKeyState::repeat;

		timer = wl_event_loop_add_timer(server->embedder_state->event_loop, [](void* data) -> int {
			auto* key_repeater = static_cast<KeyRepeater*>(data);
			ZenithServer::instance()->embedder_state->send_key_event(key_repeater->repeated_event);
			wl_event_source_timer_update(key_repeater->timer, key_repeater->rate);
			return 0;
		}, this);
		wl_event_source_timer_update(timer, delay);
	});
}

void KeyRepeater::cancel_repeat() {
	auto* server = ZenithServer::instance();
	server->embedder_state->callable_queue.enqueue([this] {
		if (timer != nullptr) {
			wl_event_source_remove(timer);
			timer = nullptr;
		}
	});
}
