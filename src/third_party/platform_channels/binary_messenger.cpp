#include <iostream>
#include "binary_messenger.hpp"
#include "incoming_message_dispatcher.hpp"

using FlutterDesktopMessengerRef = BinaryMessenger*;

bool
send_message_with_reply(FlutterEngine engine, const char* channel, const uint8_t* message, const size_t message_size,
                        const FlutterDataCallback reply, void* user_data) {
	FlutterPlatformMessageResponseHandle* response_handle = nullptr;
	if (reply != nullptr && user_data != nullptr) {
		FlutterEngineResult result =
			  FlutterPlatformMessageCreateResponseHandle(engine, reply, user_data, &response_handle);
		if (result != kSuccess) {
			std::cerr << "ERROR: Failed to create response handle\n";
			return false;
		}
	}

	FlutterPlatformMessage platform_message = {
		  sizeof(FlutterPlatformMessage),
		  channel,
		  message,
		  message_size,
		  response_handle,
	};

	FlutterEngineResult message_result = FlutterEngineSendPlatformMessage(engine, &platform_message);
	if (response_handle != nullptr) {
		FlutterPlatformMessageReleaseResponseHandle(engine, response_handle);
	}
	return message_result == kSuccess;
}

void ForwardToHandler(FlutterDesktopMessengerRef messenger,
                      const FlutterDesktopMessage* message,
                      void* user_data) {
	auto* response_handle = message->response_handle;
	BinaryReply reply_handler = [messenger, response_handle](
		  const uint8_t* reply,
		  size_t reply_size) mutable {
		if (!response_handle) {
			std::cerr << "Error: Response can be set only once. Ignoring "
			             "duplicate response."
			          << std::endl;
			return;
		}
		FlutterEngineSendPlatformMessageResponse(messenger->GetEngine(), response_handle, reply,
		                                         reply_size);
		// The engine frees the response handle once
		// FlutterDesktopSendMessageResponse is called.
		response_handle = nullptr;
	};

	const BinaryMessageHandler& message_handler =
		  *static_cast<BinaryMessageHandler*>(user_data);

	message_handler(message->message, message->message_size,
	                std::move(reply_handler));
}

void BinaryMessenger::Send(const std::string& channel,
                           const uint8_t* message,
                           size_t message_size,
                           BinaryReply reply) const {
	if (reply == nullptr) {
		send_message_with_reply(engine_, channel.c_str(), message, message_size, nullptr, nullptr);
		return;
	}

	struct Captures {
		BinaryReply reply;
	};
	auto captures = new Captures();
	captures->reply = reply;

	auto message_reply = [](const uint8_t* data, size_t data_size,
	                        void* user_data) {
		auto captures = reinterpret_cast<Captures*>(user_data);
		captures->reply(data, data_size);
		delete captures;
	};
	bool result = send_message_with_reply(engine_, channel.c_str(), message, message_size, message_reply, captures);
	if (!result) {
		delete captures;
	}
}

void BinaryMessenger::SetMessageHandler(const std::string& channel,
                                        BinaryMessageHandler handler) {
	if (!handler) {
		handlers_.erase(channel);
		message_dispatcher->SetMessageCallback(channel, nullptr, nullptr);
		return;
	}
	// Save the handler, to keep it alive.
	handlers_[channel] = std::move(handler);
	BinaryMessageHandler* message_handler = &handlers_[channel];
	// Set an adaptor callback that will invoke the handler.
	message_dispatcher->SetMessageCallback(channel, ForwardToHandler, message_handler);
}

void BinaryMessenger::SetMessageDispatcher(IncomingMessageDispatcher* message_dispatcher) {
	this->message_dispatcher = message_dispatcher;
}

FlutterEngine BinaryMessenger::GetEngine() {
	return engine_;
}

void BinaryMessenger::SetEngine(FlutterEngine engine) {
	engine_ = engine;
}
