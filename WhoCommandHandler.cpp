#include "WhoCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <sstream>

WhoCommandHandler::WhoCommandHandler(Server& server) : CommandHandler(server) {}

//should have only 1 param: channel
//if not channel, error
//else show info
void WhoCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		client.send("461 " + client.getNickname() + " WHO :Not enough parameters\r\n");
		return;
	}
    Channel *channel = server.getChannel(message.getParams()[0]);
    if (!channel) {
        client.send("403 " + client.getNickname() + " WHO :No such channel\r\n");
        return;
    }
    if (channel->getUserCount() < 1) {
        client.send("403 " + client.getNickname() + " WHO :Channel is empty\r\n");
    }
    std::stringstream ss;
    ss << "353 " << client.getNickname() << " = " << channel->getName() << " :";
    vector<Client*> clients = channel->getClients();
    // Iterate through the clients in the channel
    std::vector<Client*>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        Client* cliente = *it;
        ss << " " << cliente->getNickname() << (channel->isOperator(cliente) ? "(op)" : "");
    }
    client.send(ss.str());
    client.send("315 " + client.getNickname() + " " + channel->getName() + " :End of WHO list.\r\n");
}
