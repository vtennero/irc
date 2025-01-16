#include "UserCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>

UserCommandHandler::UserCommandHandler(Server& server) : CommandHandler(server) {}


void UserCommandHandler::handle(Client& client, const Message& message) {
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << client.getFd() << endl;

	if (client.isRegistered()) {
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Client already registered, preventing reregistration" << endl;

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

    // Store both username and realname
    client.setUsername(message.getParams()[0]);
    // The realname is the last parameter (after the colon)
    if (message.getParams().size() >= 4) {
        client.setRealname(message.getParams()[3]);
    }

	cout << "Client set username to: " << client.getUsername() << endl;

if (client.isAuthenticated() && !client.getNickname().empty()) {
    cout << "Client fully registered, sending welcome" << endl;
    client.setRegistered(true);
    sendWelcomeMessages(client);
} else {
    cout << "Registration incomplete - Auth: " << client.isAuthenticated()
              << ", Nick: " << !client.getNickname().empty() << endl;
}
}

void UserCommandHandler::sendWelcomeMessages(Client& client)
{
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Sending welcome messages to client" << endl;


	string nickname = client.getNickname();
	string username = client.getUsername();
	string hostname = client.getHostname();

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
