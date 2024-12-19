#include "NickCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>

void NickCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		client.send("431 :No nickname given\r\n");
		return;
	}

	std::string newNick = message.getParams()[0];

	if (!isValidNickname(newNick)) {
		client.send("432 " + newNick + " :Erroneous nickname\r\n");
		return;
	}

	if (server.isNicknameInUse(newNick)) {
		client.send("433 " + newNick + " :Nickname is already in use\r\n");
		return;
	}

	std::string oldNick = client.getNickname();
	client.setNickname(newNick);
	std::cout << "Client changed nickname from " << oldNick << " to: " << newNick << std::endl;

	if (client.isAuthenticated() && !client.getUsername().empty()) {
		client.setRegistered(true);
		client.send("001 " + newNick + " :Welcome to the Internet Relay Network "
				   + newNick + "!" + client.getUsername() + "@" + client.getHostname() + "\r\n");
	}
}

bool NickCommandHandler::isValidNickname(const std::string& nick)
{
	// Check length (typical IRC limit is 9)
	if (nick.empty() || nick.length() > 9)
		return false;

	// Check first character
	if (!isalpha(nick[0]) && nick[0] != '[' && nick[0] != ']' &&
		nick[0] != '\\' && nick[0] != '`' && nick[0] != '_' &&
		nick[0] != '^' && nick[0] != '{' && nick[0] != '|')
		return false;

	// Check rest of the nickname
	for (size_t i = 1; i < nick.length(); i++) {
		if (!isalnum(nick[i]) && nick[i] != '-' && nick[i] != '_')
			return false;
	}

	return true;
}
