#include "QuitCommandHandler.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

void QuitCommandHandler::handle(Client& client, const Message& message) {
	std::string quitMessage = "Client quit";
	if (!message.getParams().empty()) {
		quitMessage = message.getParams()[0];
	}

	// Construct the quit message
	std::ostringstream quitMsg;
	quitMsg << ":" << client.getNickname() << "!" << client.getUsername() << "@" << client.getHostname()
			<< " QUIT :" << quitMessage << "\r\n";

	// Log the quit message
	std::cout << "Client " << client.getNickname() << " quit: " << quitMessage << std::endl;

	// Send error message before closing
	client.send("ERROR :Closing Link: " + client.getHostname() + " (Client Quit)\r\n");

	// Broadcast quit message to all other clients
	server.broadcastMessage(quitMsg.str(), &client);

	// Remove client from server and close connection
	server.removeClient(client.getFd());
	close(client.getFd());
}
