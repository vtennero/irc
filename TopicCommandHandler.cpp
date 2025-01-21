#include "TopicCommandHandler.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Channel.hpp"

void TopicCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		client.send("461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
		return;
	}
	Channel *channel = server.getChannel(message.getParams()[0]);
	if (!channel) {
		client.send("403 " + client.getNickname() + " " + message.getParams()[0] + " : Channel doesn't exist\r\n");
		return;
	}
    //check if channel exists
    //check if message exists
    //check if mode is +t
    //check if client is operator
    //set topic
    //broadcast topic message to channel
//check for msg, if no msg, display topic
    if (message.getParams().size() == 1) {
	    //channel setter doesn't get to see, why?
        client.send("Topic of " + message.getParams()[0] + " is " + channel->getTopic() + "\r\n");
    }
    if (!channel->hasClient(&client)) {
        client.send("442 " + client.getNickname() + " " + message.getParams()[0] + " : You're not on that channel\r\n");
        return;
    }
    if (channel->checkMode('t') && (!channel->isOperator(&client))) {
        client.send("482 " + client.getNickname() + " " + message.getParams()[0] + " : You're not a channel operator\r\n");
        return;
    }
    channel->setTopic(message.getParams()[1]);
	channel->broadcastMessage(":" + client.getNickname() + " TOPIC " + channel->getName() + " :" + message.getParams()[1] + "\r\n", &client);
}
