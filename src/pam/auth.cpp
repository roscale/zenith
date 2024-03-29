#include "auth.hpp"

#include <iostream>
#include <security/pam_appl.h>
#include <unistd.h>
#include <cstring>
#include <climits>

struct ConversationData {
	const std::string& password;
	std::string message;
};

static AuthenticationResponse authenticate(const std::string& user, const std::string& password);

static int converse(int n, const struct pam_message** msg, struct pam_response** resp, void* data);

static void delay_fn(int retval, unsigned usec_delay, void* appdata_ptr);

AuthenticationResponse authenticate_current_user(const std::string& password) {
	char user[LOGIN_NAME_MAX];
	if (getlogin_r(user, sizeof(user)) != 0) {
		return AuthenticationResponse{false, {}};
	}
	return authenticate(user, password);
}

static AuthenticationResponse authenticate(const std::string& user, const std::string& password) {
	int retval;
	ConversationData conversation_data = {password, {}};

	struct pam_conv conv = {
		  converse,
		  (void*) &conversation_data,
	};

	for (size_t i = 0; i < 10; i++) {
		pam_handle_t* pam_handle = nullptr;
		retval = pam_start("zenith", user.c_str(), &conv, &pam_handle);

		if (retval == PAM_SUCCESS) {
			// Disable fail delay.
			retval = pam_set_item(pam_handle, PAM_FAIL_DELAY, (void*) delay_fn);
		}
		if (retval == PAM_SUCCESS) {
			// Is user really user?
			retval = pam_authenticate(pam_handle, 0);
		}
		if (retval == PAM_SUCCESS) {
			// Permitted access?
			retval = pam_acct_mgmt(pam_handle, 0);
		}

		pam_end(pam_handle, retval);

		if (retval == PAM_SUCCESS) {
			break;
		}
	}

	return AuthenticationResponse{retval == PAM_SUCCESS, conversation_data.message};
}


static int converse(int n, const struct pam_message** msg, struct pam_response** resp, void* data) {
	ConversationData& conversation_data = *static_cast<ConversationData*>(data);
	const std::string& password = conversation_data.password;
	std::string& message_out = conversation_data.message;

	if (n <= 0 || n > PAM_MAX_NUM_MSG) {
		return PAM_CONV_ERR;
	}
	pam_response* responses = static_cast<pam_response*>(calloc(n, sizeof *responses));
	if (responses == nullptr) {
		return PAM_BUF_ERR;
	}

	for (int i = 0; i < n; i++) {
		const pam_message* message = msg[i];
		pam_response& response = responses[i];

		response.resp_retcode = 0;
		response.resp = nullptr;

		switch (message->msg_style) {
			case PAM_PROMPT_ECHO_OFF:
			case PAM_PROMPT_ECHO_ON:
				response.resp = strdup(password.c_str());
				if (response.resp == nullptr) {
					goto fail;
				}
				break;
			case PAM_ERROR_MSG: {
				std::cerr << message->msg;
				size_t len = strlen(message->msg);
				if (len > 0 && message->msg[len - 1] != '\n') {
					std::cerr << '\n';
				}
				break;
			}
			case PAM_TEXT_INFO: {
				message_out = message->msg;

				std::cout << message->msg;
				size_t len = strlen(message->msg);
				if (len > 0 && message->msg[len - 1] != '\n') {
					std::cout << '\n';
				}
				break;
			}
			default:
				goto fail;
		}
	}
	*resp = responses;
	return PAM_SUCCESS;

	fail:
	for (int i = 0; i < n; i++) {
		pam_response& response = responses[i];
		if (response.resp != nullptr) {
			free(response.resp);
		}
	}
	free(responses);
	*resp = nullptr;
	return PAM_CONV_ERR;
}

static void delay_fn(int retval, unsigned usec_delay, void* appdata_ptr) {
	// No delay because we don't want to block the main thread.
	// https://man7.org/linux/man-pages/man3/pam_fail_delay.3.html
}
