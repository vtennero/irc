#include "UserCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>

void UserCommandHandler::handle(Client& client, const Message& message) {
	std::cout << "[DEBUG] function handle called for client fd: " << client.getFd() << std::endl;

	if (client.isRegistered()) {
		std::cout << "[DEBUG] Client already registered, preventing reregistration" << std::endl;
		client.send("462 " + client.getNickname() + " :You may not reregister\r\n");
		return;
	}

	if (!client.getUsername().empty()) {
    client.send("462 " + client.getNickname() + " :Cannot change username after it's set\r\n");
    return;
}

	if (message.getParams().size() < 4) {
		client.send("461 " + client.getNickname() + " USER :Not enough parameters\r\n");
		return;
	}

	client.setUsername(message.getParams()[0]);
	client.setRealname(message.getParams()[3]);
	std::cout << "Client set username to: " << client.getUsername() << std::endl;

if (client.isAuthenticated() && !client.getNickname().empty()) {
    std::cout << "Client fully registered, sending welcome" << std::endl;
    client.setRegistered(true);
    sendWelcomeMessages(client);
} else {
    std::cout << "Registration incomplete - Auth: " << client.isAuthenticated()
              << ", Nick: " << !client.getNickname().empty() << std::endl;
}
}

void UserCommandHandler::sendWelcomeMessages(Client& client)
{
	std::cout << "[DEBUG] Sending welcome messages to client" << std::endl;


	std::string nickname = client.getNickname();
	std::string username = client.getUsername();
	std::string hostname = client.getHostname();

	// 001 RPL_WELCOME
	client.send("001 " + nickname + " :Welcome to the Internet Relay Network "
				+ nickname + "!" + username + "@" + hostname + "\r\n");

	// 002 RPL_YOURHOST
	client.send("002 " + nickname + " :Your host is " + server.getServerName()
				+ ", running version " + server.getVersion() + "\r\n");

	// 003 RPL_CREATED
	client.send("003 " + nickname + " :This server was created "
				+ server.getCreationDate() + "\r\n");

	// 004 RPL_MYINFO
	client.send("004 " + nickname + " " + server.getServerName() + " "
				+ server.getVersion() + " <available user modes> <available channel modes>\r\n");
}
