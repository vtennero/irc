#include "QuitCommandHandler.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>









void QuitCommandHandler::handle(Client& client, const Message& message) {
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << client.getFd() << endl;

	string quitMessage = "Client quit";
	if (!message.getParams().empty()) {
		quitMessage = message.getParams()[0];
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Custom quit message received: " << quitMessage << endl;
	}

	// Construct the quit message
	ostringstream quitMsg;
	quitMsg << ":" << client.getNickname() << "!" << client.getUsername() << "@" << client.getHostname()
			<< " QUIT :" << quitMessage << "\r\n";
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Constructed quit message: " << quitMsg.str() << endl;


	// Log the quit message
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Client " << client.getNickname() << " quit: " << quitMessage << endl;


	// Send error message before closing
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Sending error message to client" << endl;

	client.send("ERROR :Closing Link: " + client.getHostname() + " (Client Quit)\r\n");

	// Broadcast quit message to all other clients
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Broadcasting quit message to other clients" << endl;

	server.broadcastMessage(quitMsg.str(), &client);

	// Remove client from server and close connection
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Removing client from server" << endl;

	server.removeClient(client.getFd());
cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Closing client connection fd: " << client.getFd() << endl;

	close(client.getFd());
}
