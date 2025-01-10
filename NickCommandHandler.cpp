#include "NickCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>

void NickCommandHandler::handle(Client& client, const Message& message) {
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << client.getFd() << endl;


	if (message.getParams().empty()) {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " No nickname provided" << endl;

		client.send("431 " + client.getNickname() + " :No nickname given\r\n");
		// client.send("431 :No nickname given\r\n");
		return;
	}

	string newNick = message.getParams()[0];
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Requested nickname: " << newNick << endl;

	if (!isValidNickname(newNick)) {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Invalid nickname format" << endl;
		client.send("432 " + newNick + " :Erroneous nickname\r\n");
		return;
	}

	if (server.isNicknameInUse(newNick)) {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Nickname already in use" << endl;

		client.send("433 " + newNick + " :Nickname is already in use\r\n");
		return;
	}

	string oldNick = client.getNickname();
	client.setNickname(newNick);
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Nickname changed from '" << oldNick << "' to '" << newNick << "'" << endl;


	// Send NICK change notification to client
	string nickMsg = ":" + oldNick + "!" + client.getUsername() + "@" +
					client.getHostname() + " NICK :" + newNick + "\r\n";
	client.send(nickMsg);

	// Only send welcome messages if this completes registration
	if (!client.isRegistered() && client.isAuthenticated() && !client.getUsername().empty()) {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Client fully registered, sending welcome message" << endl;

		client.setRegistered(true);
		client.send("001 " + newNick + " :Welcome to the Internet Relay Network " +
				   newNick + "!" + client.getUsername() + "@" + client.getHostname() + "\r\n");
	}
}

bool NickCommandHandler::isValidNickname(const string& nick)
{
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with nickname: " << nick << endl;


	if (nick.empty() || nick.length() > 9) {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Nickname length invalid: " << nick.length() << " chars" << endl;

		return false;
	}

	if (!isalpha(nick[0]) && nick[0] != '[' && nick[0] != ']' &&
		nick[0] != '\\' && nick[0] != '`' && nick[0] != '_' &&
		nick[0] != '^' && nick[0] != '{' && nick[0] != '|') {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Invalid first character: " << nick[0] << endl;

		return false;
	}

	for (size_t i = 1; i < nick.length(); i++) {
		if (!isalnum(nick[i]) && nick[i] != '-' && nick[i] != '_') {
			cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Invalid character at position " << i << ": " << nick[i] << endl;

			return false;
		}
	}
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Nickname validation successful" << endl;

	return true;
}
