#include "UserCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>

UserCommandHandler::UserCommandHandler(Server& server) : CommandHandler(server)
{}

void UserCommandHandler::handle(Client& client, const Message& message) {
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << client.getFd() << endl;

	if (!validateRegistrationStatus(client)) {
		return;
	}

	if (!validateMessageParameters(client, message)) {
		return;
	}

	processUserInformation(client, message);
	checkAndCompleteRegistration(client);
}

bool UserCommandHandler::validateRegistrationStatus(Client& client) {
	if (client.isRegistered()) {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Client already registered, preventing reregistration" << endl;
		client.send("462 " + client.getNickname() + " :You may not reregister\r\n");
		return false;
	}

	if (!client.getUsername().empty()) {
		client.send("462 " + client.getNickname() + " :Cannot change username after it's set\r\n");
		return false;
	}

	return true;
}

bool UserCommandHandler::validateMessageParameters(Client& client, const Message& message) {
	if (message.getParams().size() < 4) {
		client.send("461 " + client.getNickname() + " USER :Not enough parameters\r\n");
		return false;
	}
	return true;
}

void UserCommandHandler::processUserInformation(Client& client, const Message& message) {
	client.setUsername(message.getParams()[0]);
	if (message.getParams().size() >= 4) {
		client.setRealname(message.getParams()[3]);
	}
	cout << "Client set username to: " << client.getUsername() << endl;
}

void UserCommandHandler::checkAndCompleteRegistration(Client& client) {
	if (client.isAuthenticated() && !client.getNickname().empty()) {
		cout << "Client fully registered, sending welcome" << endl;
		client.setRegistered(true);
		sendWelcomeMessages(client);
	} else {
		cout << "Registration incomplete - Auth: " << client.isAuthenticated()
				  << ", Nick: " << !client.getNickname().empty() << endl;
	}
}

void UserCommandHandler::sendWelcomeMessages(Client& client) {
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Sending welcome messages to client" << endl;

	const string& nickname = client.getNickname();
	const string& username = client.getUsername();
	const string& hostname = client.getHostname();

	sendWelcomeMessage(client, nickname, username, hostname);
	sendHostInfo(client, nickname);
	sendServerCreationInfo(client, nickname);
	sendServerCapabilities(client, nickname);
}

void UserCommandHandler::sendWelcomeMessage(Client& client, const string& nickname, const string& username, const string& hostname)
{
	client.send("001 " + nickname + " :Welcome to the Internet Relay Network " + nickname + "!" + username + "@" + hostname + "\r\n");
}

void UserCommandHandler::sendHostInfo(Client& client, const string& nickname) {
	client.send("002 " + nickname + " :Your host is " + server.getServerName()
				+ ", running version " + server.getVersion() + "\r\n");
}

void UserCommandHandler::sendServerCreationInfo(Client& client, const string& nickname) {
	client.send("003 " + nickname + " :This server was created "
				+ server.getCreationDate() + "\r\n");
}

void UserCommandHandler::sendServerCapabilities(Client& client, const string& nickname) {
	client.send("004 " + nickname + " " + server.getServerName() + " "
				+ server.getVersion() + " <available user modes> <available channel modes>\r\n");
}
