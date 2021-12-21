#pragma once

#include "third_party/embedder.h"
#include <string>
#include <functional>
#include <map>

class IncomingMessageDispatcher;

using BinaryReply = std::function<void(const uint8_t* reply, size_t reply_size)>;

using BinaryMessageHandler = std::function<void(const uint8_t* message, size_t message_size, BinaryReply reply)>;

using BinaryMessageHandler = std::function<void(const uint8_t* message, size_t message_size, BinaryReply reply)>;

class BinaryMessenger {
public:
	void Send(const std::string& channel,
	          const uint8_t* message,
	          size_t message_size,
	          BinaryReply reply = nullptr) const;

	void SetMessageHandler(const std::string& channel,
	                       BinaryMessageHandler handler);

	// My own methods

	void SetMessageDispatcher(IncomingMessageDispatcher* message_dispatcher);

	FlutterEngine GetEngine();

	void SetEngine(FlutterEngine engine);

private:
	// Handle for interacting with the C API.
	FlutterEngine engine_;

	IncomingMessageDispatcher* message_dispatcher;

	// A map from channel names to the BinaryMessageHandler that should be called
	// for incoming messages on that channel.
	std::map<std::string, BinaryMessageHandler> handlers_;
};
