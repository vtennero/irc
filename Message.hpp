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

class Message
{
	private:
		string prefix;
		string command;
		vector<string> params;
		string rawMessage;  // Store the original raw message
		string token;  // Add token field for PING/PONG messages
		void parseMessage(string& msg);
		void parsePrefix(string& msg);
		void parseCommand(string& msg);
		void parseParameters(string& msg);
		void parseTrailingParameter(const string& msg);
		void parseRegularParameter(string& msg);
		void logParameter(const string& param, const string& type) const;
		void parseToken();  // Add method to extract token

	public:

		Message(const string& rawMessage);
		string getCommand() const;
		string getPrefix() const;
		const vector<string>& getParams() const;
		const string& getRawMessage() const;
		const string& getToken() const;

};

#endif
