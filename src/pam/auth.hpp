#pragma once

#include <string>
#include <vector>
#include <optional>

struct AuthenticationResponse {
	bool success = false;
	std::string message;
};

AuthenticationResponse authenticate_current_user(const std::string& password);
