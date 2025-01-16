#include "Message.hpp"
#include <sstream>
#include <iostream>

string Message::getCommand() const {
	return command;
}

string Message::getPrefix() const {
	return prefix;
}

const vector<string>& Message::getParams() const {
	return params;
}

const string& Message::getRawMessage() const {
	return rawMessage;
}

const string& Message::getToken() const {
	return token;
}


Message::Message(const string& rawMessage)
	: rawMessage(rawMessage)
{
	cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with raw message: " << rawMessage << endl;

	string msg = rawMessage;
	parseMessage(msg);
	parseToken();
}

void Message::parseMessage(string& msg)
{
	parsePrefix(msg);
	parseCommand(msg);
	if (!msg.empty()) {
		parseParameters(msg);
	}
}

void Message::parsePrefix(string& msg)
{
	if (!msg.empty() && msg[0] == ':') {
		size_t pos = msg.find(' ');
		if (pos != string::npos) {
			prefix = msg.substr(1, pos - 1);  // Store the prefix without the :
			cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Extracted prefix: " << prefix << endl;
			msg = msg.substr(pos + 1);
		}
	}
}

void Message::parseCommand(string& msg)
{
	size_t pos = msg.find(' ');
	if (pos != string::npos) {
		command = msg.substr(0, pos);
		cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Extracted command: " << command << endl;
		msg = msg.substr(pos + 1);
	} else {
		command = msg;
		msg.clear();
	}
}

void Message::parseParameters(string& msg)
{
	while (!msg.empty()) {
		if (msg[0] == ':') {
			parseTrailingParameter(msg);
			break;
		}
		parseRegularParameter(msg);
	}
}

void Message::parseTrailingParameter(const string& msg)
{
	string param = msg.substr(1);
	params.push_back(param);
	logParameter(param, "trailing");
}

void Message::parseRegularParameter(string& msg)
{
	size_t pos = msg.find(' ');
	if (pos != string::npos) {
		string param = msg.substr(0, pos);
		if (!param.empty()) {
			params.push_back(param);
			logParameter(param, "");
		}
		msg = msg.substr(pos + 1);
	} else {
		if (!msg.empty()) {
			params.push_back(msg);
			logParameter(msg, "final");
		}
		msg.clear();
	}
}

void Message::logParameter(const string& param, const string& type) const
{
	string logMessage = type.empty() ? "Added parameter: " :
					   ("Added " + type + " parameter: ");
	cout << SKY_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " " << logMessage << param << endl;
}

void Message::parseToken()
{
	if (command == "PING" || command == "PONG") {
		token = params.empty() ? "" : params[0];
	}
}
