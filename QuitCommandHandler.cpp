#include "QuitCommandHandler.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

void QuitCommandHandler::handle(Client& client, const Message& message) {
	std::cout << "[DEBUG] function handle called for client fd: " << client.getFd() << std::endl;

	std::string quitMessage = "Client quit";
	if (!message.getParams().empty()) {
		quitMessage = message.getParams()[0];
		std::cout << "[DEBUG] Custom quit message received: " << quitMessage << std::endl;
	}

	// Construct the quit message
	std::ostringstream quitMsg;
	quitMsg << ":" << client.getNickname() << "!" << client.getUsername() << "@" << client.getHostname()
			<< " QUIT :" << quitMessage << "\r\n";
	std::cout << "[DEBUG] Constructed quit message: " << quitMsg.str();

	// Log the quit message
	std::cout << "[DEBUG] Client " << client.getNickname() << " quit: " << quitMessage << std::endl;

	// Send error message before closing
	std::cout << "[DEBUG] Sending error message to client" << std::endl;
	client.send("ERROR :Closing Link: " + client.getHostname() + " (Client Quit)\r\n");

	// Broadcast quit message to all other clients
	std::cout << "[DEBUG] Broadcasting quit message to other clients" << std::endl;
	server.broadcastMessage(quitMsg.str(), &client);

	// Remove client from server and close connection
	std::cout << "[DEBUG] Removing client from server" << std::endl;
	server.removeClient(client.getFd());
	std::cout << "[DEBUG] Closing client connection fd: " << client.getFd() << std::endl;
	close(client.getFd());
}
