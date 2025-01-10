#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <iostream>
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class Message {
private:
	string prefix;
	string command;
	vector<string> params;
	string rawMessage;  // Store the original raw message
	string token;  // Add token field for PING/PONG messages

public:
	Message(const string& rawMessage);
	string getCommand() const { return command; }
	string getPrefix() const { return prefix; }
	const vector<string>& getParams() const { return params; }
	const string& getRawMessage() const { return rawMessage; }
	const string& getToken() const { return token; }  // Add getter for token

private:
	void parseToken();  // Add method to extract token

};

#endif
