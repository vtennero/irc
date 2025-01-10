#include "Message.hpp"
#include <sstream>
#include <iostream>


void Message::parseToken()
{
	if (command == "PING" || command == "PONG") {
		token = params.empty() ? "" : params[0];
	}
}

Message::Message(const string& rawMessage)
	: rawMessage(rawMessage)  // Store the original message
{
	cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with raw message: " << rawMessage << endl;

	string msg = rawMessage;
	size_t pos = 0;

	// Handle prefix if exists (starts with :)
	if (!msg.empty() && msg[0] == ':') {
		pos = msg.find(' ');
		if (pos != string::npos) {
			prefix = msg.substr(1, pos - 1);  // Store the prefix without the :
			cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Extracted prefix: " << prefix << endl;

			msg = msg.substr(pos + 1);
		}
	}

	// Parse command
	pos = msg.find(' ');
	if (pos != string::npos) {
		command = msg.substr(0, pos);
		cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Extracted command: " << command << endl;

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
			cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Added trailing parameter: " << msg.substr(1) << endl;

			break;
		}

		pos = msg.find(' ');
		if (pos != string::npos) {
			string param = msg.substr(0, pos);
			if (!param.empty()) {
				params.push_back(param);
				cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Added parameter: " << param << endl;

			}
			msg = msg.substr(pos + 1);
		} else {
			if (!msg.empty()) {
				params.push_back(msg);
				cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Added final parameter: " << msg << endl;

			}
			break;
		}
	}
}
