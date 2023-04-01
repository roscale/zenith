#pragma once

#include "text_input_model.h"

struct TextInputClient {
	size_t transaction;
	flutter::TextInputModel model = {};
	std::string input_action;

public:
	TextInputClient(size_t transaction, std::string input_action);
};
