#include "TopicCommandHandler.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Channel.hpp"

void TopicCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		client.send("461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
		return;
	}

    //check if channel exists
    //check if message exists
    //check if mode is +t
    //check if client is operator
    //set topic
    //broadcast topic message to channel


    Channel *channel = server.getChannel(message.getParams()[0]);
	if (!channel) {
        client.send("403 " + client.getNickname() + " " + message.getParams()[0] + " : Channel doesn't exist\r\n");
        return;
	}
    if (!channel->hasClient(&client)) {
        client.send("442 " + client.getNickname() + " " + message.getParams()[0] + " : You're not on that channel\r\n");
        return;
    }
    if (!channel->isOperator(&client)) {
        client.send("482 " + client.getNickname() + " " + message.getParams()[0] + " : You're not a channel operator\r\n");
        return;
    }
    Client *target = server.findClientByNickname(message.getParams()[1]);
    if (!target || !channel->hasClient(target)) {
        client.send("401 " + client.getNickname() + " " + message.getParams()[1] + " : No such nick/channel\r\n");
        return;
    }
    if (channel->isOperator(target)) {
        client.send("482 " + client.getNickname() + " " + message.getParams()[0] + " : Can't kick another operator\r\n");
        return;
    }
    channel->removeClient(target);
    //broadcast kick message to channel.
	string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " KICK " + channel->getName() + " " + target->getNickname() + " : " + message.getParams()[2];
	channel->broadcastMessage(kickMsg);

	if (message.getParams().size() > 2) {
        channel->broadcastMessage(message.getParams()[2] + "\r\n");
	} else {
		channel->broadcastMessage("\r\n");
	}
}
