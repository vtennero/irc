#include "QuitCommandHandler.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

void QuitCommandHandler::handle(Client& client, const Message& message) {
	cout << "[DEBUG] function handle called for client fd: " << client.getFd() << endl;

	string quitMessage = "Client quit";
	if (!message.getParams().empty()) {
		quitMessage = message.getParams()[0];
		cout << "[DEBUG] Custom quit message received: " << quitMessage << endl;
	}

	// Construct the quit message
	ostringstream quitMsg;
	quitMsg << ":" << client.getNickname() << "!" << client.getUsername() << "@" << client.getHostname()
			<< " QUIT :" << quitMessage << "\r\n";
	cout << "[DEBUG] Constructed quit message: " << quitMsg.str();

	// Log the quit message
	cout << "[DEBUG] Client " << client.getNickname() << " quit: " << quitMessage << endl;

	// Send error message before closing
	cout << "[DEBUG] Sending error message to client" << endl;
	client.send("ERROR :Closing Link: " + client.getHostname() + " (Client Quit)\r\n");

	// Broadcast quit message to all other clients
	cout << "[DEBUG] Broadcasting quit message to other clients" << endl;
	server.broadcastMessage(quitMsg.str(), &client);

	// Remove client from server and close connection
	cout << "[DEBUG] Removing client from server" << endl;
	server.removeClient(client.getFd());
	cout << "[DEBUG] Closing client connection fd: " << client.getFd() << endl;
	close(client.getFd());
}
