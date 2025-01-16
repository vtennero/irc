#include "ModeCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

ModeCommandHandler::ModeCommandHandler(Server& server) : CommandHandler(server) {}


void ModeCommandHandler::handle(Client& client, const Message& message) {
	// Basic MODE implementation - just acknowledge the mode request
	if (message.getParams().empty()) {
		client.send("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
		return;
	}

	// For now, just send a generic response for user modes
	if (message.getParams()[0] == "*" || message.getParams()[0] == client.getNickname()) {
		client.send(":" + server.getServerName() + " 221 " + client.getNickname() + " +i\r\n");
	}
}
