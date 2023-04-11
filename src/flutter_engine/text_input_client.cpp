#include "text_input_client.hpp"

#include <utility>

TextInputClient::TextInputClient(flutter::MethodChannel<rapidjson::Document>& text_input_method_channel,
                                 size_t transaction,
                                 std::string input_action)
	  : text_input_method_channel(text_input_method_channel),
	    transaction(transaction),
	    input_action(std::move(input_action)) {
}

void TextInputClient::update_editing_state() {
	auto json = std::make_unique<rapidjson::Document>();
	auto& allocator = json->GetAllocator();

	json->SetArray();
	json->PushBack(transaction, allocator);

	rapidjson::Value editing_values = {};
	editing_values.SetObject();

	std::string text = model.GetText();
	rapidjson::Value text_value;
	text_value.SetString(text.c_str(), text.length(), allocator);
	editing_values.AddMember("text", text_value, allocator);

	flutter::TextRange selection = model.selection();
	editing_values.AddMember("selectionBase", selection.base(), allocator);
	editing_values.AddMember("selectionExtent", selection.extent(), allocator);

	flutter::TextRange composing_range = model.composing_range();
	editing_values.AddMember("composingBase", composing_range.base(), allocator);
	editing_values.AddMember("composingExtent", composing_range.extent(), allocator);

	json->PushBack(editing_values, allocator);

	text_input_method_channel.InvokeMethod("TextInputClient.updateEditingState", std::move(json));
}

void TextInputClient::enter() {
	if (input_action == "TextInputAction.newline") {
		add_text("\n");
	}

	auto json = std::make_unique<rapidjson::Document>();
	auto& allocator = json->GetAllocator();

	json->SetArray();
	json->PushBack(transaction, allocator);

	rapidjson::Value input_action_value;
	input_action_value.SetString(input_action.c_str(),
	                             input_action.length(), allocator);
	json->PushBack(input_action_value, allocator);

	text_input_method_channel.InvokeMethod("TextInputClient.performAction", std::move(json));
}

void TextInputClient::add_text(const std::string& text) {
	model.AddText(text);
	update_editing_state();
}

void TextInputClient::set_editing_state(const std::string& text,
                                        const flutter::TextRange& composing_range,
                                        const flutter::TextRange& selection) {

	model.SetText(text);
	model.SetComposingRange(composing_range, model.selection().extent());
	model.SetSelection(selection);
}

void TextInputClient::close_connection() {
	auto json = std::make_unique<rapidjson::Document>();
	auto& allocator = json->GetAllocator();

	json->SetArray();
	json->PushBack(transaction, allocator);

	text_input_method_channel.InvokeMethod("TextInputClient.onConnectionClosed", std::move(json));
}

void TextInputClient::backspace() {
	model.Backspace();
}
