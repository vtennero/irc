#include "Message.hpp"
#include <sstream>
#include <iostream>

Message::Message(const std::string& rawMessage) {
	std::cout << "[DEBUG] function Message constructor called with raw message: " << rawMessage << std::endl;
	std::string msg = rawMessage;
	size_t pos = 0;

	// Handle prefix if exists (starts with :)
	if (!msg.empty() && msg[0] == ':') {
		pos = msg.find(' ');
		if (pos != std::string::npos) {
			prefix = msg.substr(1, pos - 1);
			msg = msg.substr(pos + 1);
		}
	}

	// Parse command
	pos = msg.find(' ');
	if (pos != std::string::npos) {
		command = msg.substr(0, pos);
		msg = msg.substr(pos + 1);
	} else {
		command = msg;
		return;
	}

	// Parse parameters
	while (!msg.empty()) {
		// If we encounter a :, the rest is the trailing parameter
		if (msg[0] == ':') {
			params.push_back(msg.substr(1)); // Store everything after the colon
			std::cout << "[DEBUG] Added trailing parameter: " << msg.substr(1) << std::endl;
			break;
		}

		pos = msg.find(' ');
		if (pos != std::string::npos) {
			std::string param = msg.substr(0, pos);
			if (!param.empty()) {
				params.push_back(param);
				std::cout << "[DEBUG] Added parameter: " << param << std::endl;
			}
			msg = msg.substr(pos + 1);
		} else {
			if (!msg.empty()) {
				params.push_back(msg);
				std::cout << "[DEBUG] Added final parameter: " << msg << std::endl;
			}
			break;
		}
	}
}
