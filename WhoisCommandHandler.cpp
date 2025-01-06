#include "WhoisCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void WhoisCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		client.send("431 " + client.getNickname() + " :No nickname given\r\n");
		return;
	}

	string targetNick = message.getParams()[0];
	cout << "[DEBUG] WHOIS - Looking for target: " << targetNick << endl;

	Client* target = server.findClientByNickname(targetNick);
	cout << "[DEBUG] WHOIS - Target found: " << (target ? "yes" : "no") << endl;

	if (!target) {
		cout << "[DEBUG] WHOIS - Target not found, sending error" << endl;
		client.send("401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	// Send WHOIS information
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
