#pragma once

#include <src/platform_channels/event_channel.h>

template<typename T = flutter::EncodableValue>
class NewTextureStreamHandler : public flutter::StreamHandler<T> {
public:
	std::unique_ptr<flutter::EventSink<T>> sink;

protected:
	std::unique_ptr<flutter::StreamHandlerError<T>> OnListenInternal(const flutter::EncodableValue* arguments,
	                                                                 std::unique_ptr<flutter::EventSink<T>>&& events) override;

	std::unique_ptr<flutter::StreamHandlerError<T>> OnCancelInternal(const flutter::EncodableValue* arguments) override;
};

template<typename T>
std::unique_ptr<flutter::StreamHandlerError<T>>
NewTextureStreamHandler<T>::OnListenInternal(const flutter::EncodableValue* arguments,
                                             std::unique_ptr<flutter::EventSink<T>>&& events) {
	sink = std::move(events);
	return nullptr;
}

template<typename T>
std::unique_ptr<flutter::StreamHandlerError<T>>
NewTextureStreamHandler<T>::OnCancelInternal(const flutter::EncodableValue* arguments) {
	sink = nullptr;
	return nullptr;
}
