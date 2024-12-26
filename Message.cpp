#include "Message.hpp"
#include <sstream>
#include <iostream>

Message::Message(const std::string& rawMessage) {
	std::cout << "[DEBUG] function Message constructor called with raw message: " << rawMessage << std::endl;
	std::istringstream iss(rawMessage);
	std::string token;

	if (rawMessage[0] == ':') {
		iss >> prefix;
		prefix = prefix.substr(1);
		std::cout << "[DEBUG] Parsed prefix: " << prefix << std::endl;
	}

	iss >> command;
	std::cout << "[DEBUG] Parsed command: " << command << std::endl;

	while (iss >> token) {
		if (token[0] == ':') {
			std::string trailing = token.substr(1);
			std::getline(iss, token);
			trailing += token;
			params.push_back(trailing);
			std::cout << "[DEBUG] Added trailing parameter: " << trailing << std::endl;
			break;
		}
		params.push_back(token);
		std::cout << "[DEBUG] Added parameter: " << token << std::endl;
	}
}
