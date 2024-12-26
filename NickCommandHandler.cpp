#include "NickCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>

void NickCommandHandler::handle(Client& client, const Message& message) {
	std::cout << "[DEBUG] function handle called for client fd: " << client.getFd() << std::endl;

	if (message.getParams().empty()) {
		std::cout << "[DEBUG] No nickname provided" << std::endl;
		client.send("431 :No nickname given\r\n");
		return;
	}

	std::string newNick = message.getParams()[0];
	std::cout << "[DEBUG] Requested nickname: " << newNick << std::endl;

	if (!isValidNickname(newNick)) {
		std::cout << "[DEBUG] Invalid nickname format" << std::endl;
		client.send("432 " + newNick + " :Erroneous nickname\r\n");
		return;
	}

	if (server.isNicknameInUse(newNick)) {
		std::cout << "[DEBUG] Nickname already in use" << std::endl;
		client.send("433 " + newNick + " :Nickname is already in use\r\n");
		return;
	}

	std::string oldNick = client.getNickname();
	client.setNickname(newNick);
	std::cout << "[DEBUG] Nickname changed from '" << oldNick << "' to '" << newNick << "'" << std::endl;

	if (client.isAuthenticated() && !client.getUsername().empty()) {
		std::cout << "[DEBUG] Client fully registered, sending welcome message" << std::endl;
		client.setRegistered(true);
		client.send("001 " + newNick + " :Welcome to the Internet Relay Network "
				   + newNick + "!" + client.getUsername() + "@" + client.getHostname() + "\r\n");
	}
}

bool NickCommandHandler::isValidNickname(const std::string& nick)
{
	std::cout << "[DEBUG] function isValidNickname called with nickname: " << nick << std::endl;

	if (nick.empty() || nick.length() > 9) {
		std::cout << "[DEBUG] Nickname length invalid: " << nick.length() << " chars" << std::endl;
		return false;
	}

	if (!isalpha(nick[0]) && nick[0] != '[' && nick[0] != ']' &&
		nick[0] != '\\' && nick[0] != '`' && nick[0] != '_' &&
		nick[0] != '^' && nick[0] != '{' && nick[0] != '|') {
		std::cout << "[DEBUG] Invalid first character: " << nick[0] << std::endl;
		return false;
	}

	for (size_t i = 1; i < nick.length(); i++) {
		if (!isalnum(nick[i]) && nick[i] != '-' && nick[i] != '_') {
			std::cout << "[DEBUG] Invalid character at position " << i << ": " << nick[i] << std::endl;
			return false;
		}
	}

	std::cout << "[DEBUG] Nickname validation successful" << std::endl;
	return true;
}
