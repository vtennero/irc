#include "InviteCommandHandler.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Channel.hpp"

void InviteCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().size() < 2) {
		client.send("461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
		return;
	}

    //check if channel exists
    //check if message exists
    //check if client is operator
    //check if target client exists
    //send DM to target client
    //Add Client to invite list of the channel


    Channel *channel = server.getChannel(message.getParams()[0]);
	if (!channel) {
        client.send("403 " + client.getNickname() + " " + message.getParams()[0] + " : Channel doesn't exist\r\n");
        return;
	}
    if (!channel->hasClient(&client)) {
        client.send("442 " + client.getNickname() + " " + message.getParams()[0] + " : You're not on that channel\r\n");
        return;
    }
    if (!channel->getMode('t') && (!channel->isOperator(&client))) {
        client.send("482 " + client.getNickname() + " " + message.getParams()[0] + " : You're not a channel operator\r\n");
        return;
    }
    client* target = server.findClientByNickname(message.getParams()[1]);
    if (!target) {
        client.send("401 " + client.getNickname() + " " + message.getParams()[1] + " : No such nick\r\n");
        return;
    }
    //if target alr in channel, msg client that target alr in channel, else DM target to ask to /join
    if (channel->hasClient(target)) {
        client.send("443 " + client.getNickname() + " " + message.getParams()[1] + " " + channel->getName() + " : is already on channel\r\n");
        channel->addInvite(target);
    } else {
        target->send(":" + client.getNickname() + " has invited you to join " + channel->getName() + "\r\n");
        target->send("Enter /join " + channel->getName() + " to join the channel!\r\n");
    }
    channel->addInvite(target);
}