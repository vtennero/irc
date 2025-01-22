#include "TopicCommandHandler.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Channel.hpp"
#include <sstream>

//[0] must be channel, [1] onwards all considered as topic string
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
    //set topic
    //broadcast topic message to channel
//check for msg, if no msg, DM topic to client
    //check if client on channel
    if (!channel->hasClient(&client)) {
        client.send("442 " + client.getNickname() + " " + message.getParams()[0] + " : You're not on that channel\r\n");
        return;
        //check if any args, display topic in DM if non
    }
    if (message.getParams().size() == 1) {
        client.send("Topic of " + message.getParams()[0] + " is " + channel->getTopic() + "\r\n");
        return;
    }
    //check if client is operator if channel mode is +t
    if (channel->checkMode('t')) {
        if (!client.isAuthenticated()) {
			client.send("481 " + client.getNickname() + " " + message.getParams()[0] + " : You're not authenticated\r\n");
			return;
		}
        if (!channel->isOperator(&client)) {
            client.send("482 " + client.getNickname() + " " + message.getParams()[0] + " : You're not a channel operator\r\n");
            return;
        }
    }
    //combine all remaining args into a single string
    std::ostringstream newTopic;
    const vector<string>& params = message.getParams();

    for (size_t i = 1; i < params.size(); ++i) {
        newTopic << params[i];
        if (i != params.size() - 1) {
            newTopic << " ";
        }
    }
    channel->setTopic(newTopic.str());
	channel->broadcastMessage(":" + client.getNickname() + " TOPIC " + channel->getName() + " :" + newTopic.str() + "\r\n", &client);
}
