#include "text_input_client.hpp"

#include <utility>

TextInputClient::TextInputClient(size_t transaction, std::string input_action)
	  : transaction(transaction),
	    input_action(std::move(input_action)) {
}
