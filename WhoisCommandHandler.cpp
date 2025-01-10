#include "WhoisCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void WhoisCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		sendNoNicknameError(client);
		return;
	}

	string targetNick = message.getParams()[0];
	Client* target = findAndValidateTarget(client, targetNick);

	if (target) {
		sendWhoisInfo(client, target);
	}
}

void WhoisCommandHandler::sendNoNicknameError(Client& client) const {
	client.send("431 " + client.getNickname() + " :No nickname given\r\n");
}


void WhoisCommandHandler::sendNoSuchNickError(Client& client, const string& targetNick) const {
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " WHOIS - Target not found, sending error" << endl;

	client.send("401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
}

Client* WhoisCommandHandler::findAndValidateTarget(Client& client, const string& targetNick) const {
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " WHOIS - Looking for target: " << targetNick << endl;


	Client* target = server.findClientByNickname(targetNick);
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " WHOIS - Target found: " << (target ? "yes" : "no") << endl;

	if (!target) {
		sendNoSuchNickError(client, targetNick);
	}

	return target;
}

void WhoisCommandHandler::sendWhoisInfo(Client& client, Client* target) const {
	// 311 RPL_WHOISUSER
	client.send("311 " + client.getNickname() + " " + target->getNickname() + " " +
				target->getUsername() + " " + target->getHostname() + " * :" +
				target->getUsername() + "\r\n");

	// 312 RPL_WHOISSERVER
	client.send("312 " + client.getNickname() + " " + target->getNickname() + " " +
				server.getServerName() + " :IRC Server\r\n");

	// 318 RPL_ENDOFWHOIS
	client.send("318 " + client.getNickname() + " " + target->getNickname() + " :End of /WHOIS list\r\n");
}
