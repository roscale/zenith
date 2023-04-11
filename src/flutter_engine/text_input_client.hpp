#pragma once

#include "text_input_model.h"
#include "method_channel.h"
#include "document.h"

class TextInputClient {
	flutter::MethodChannel<rapidjson::Document>& text_input_method_channel;
	size_t transaction;
	flutter::TextInputModel model = {};
	std::string input_action;

public:
	TextInputClient(flutter::MethodChannel<rapidjson::Document>& text_input_method_channel,
	                size_t transaction,
	                std::string input_action);

	void update_editing_state();

	void enter();

	void backspace();

	void add_text(const std::string& text);

	void set_editing_state(const std::string& text,
						   const flutter::TextRange& composing_range,
						   const flutter::TextRange& selection);

	void close_connection();
};
