#include "PrivmsgCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include <iostream>


void PrivmsgCommandHandler::handle(Client& client, const Message& message) {
	if (!client.isRegistered()) {
		client.send("451 " + client.getNickname() + " :You have not registered\r\n");
		return;
	}

	// Check parameters
	if (message.getParams().empty()) {
		client.send("411 " + client.getNickname() + " :No recipient given (PRIVMSG)\r\n");
		return;
	}
	if (message.getParams().size() < 2) {
		client.send("412 " + client.getNickname() + " :No text to send\r\n");
		return;
	}

	std::string target = message.getParams()[0];
	std::string msgText = message.getParams()[1];

	// Handle channel or private messages
	if (target[0] == '#') {
		sendChannelMessage(client, target, msgText);
	} else {
		sendPrivateMessage(client, target, msgText);
	}
}


std::string PrivmsgCommandHandler::formatMessage(const Client& sender, const std::string& target, const std::string& message) {
    // Ensure proper IRC message format with \r\n
    return ":" + sender.getNickname() +
           "!" + sender.getUsername() +
           "@" + sender.getHostname() +
           " PRIVMSG " + target +
           " :" + message + "\r\n";
}

void PrivmsgCommandHandler::sendPrivateMessage(Client& sender, const std::string& targetNick, const std::string& message) {
    // Remove trailing whitespace from message using C++98 methods
    std::string trimmedMessage = message;
    size_t endpos = trimmedMessage.find_last_not_of(" \r\n");
    if (endpos != std::string::npos) {
        trimmedMessage = trimmedMessage.substr(0, endpos + 1);
    } else {
        trimmedMessage.clear();
    }

    // Rest of the validation
    if (trimmedMessage.empty()) {
        sender.send("412 " + sender.getNickname() + " :No text to send\r\n");
        return;
    }

    std::cout << "[DEBUG] PRIVMSG - Attempting to send from " << sender.getNickname()
              << " to " << targetNick << std::endl;
    std::cout << "[DEBUG] PRIVMSG - Message content: " << trimmedMessage << std::endl;

    Client* target = server.findClientByNickname(targetNick);
    if (!target) {
        sender.send("401 " + sender.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

    if (!target->isRegistered()) {
        sender.send("401 " + sender.getNickname() + " " + targetNick + " :User not registered\r\n");
        return;
    }

    std::string formattedMsg = formatMessage(sender, targetNick, trimmedMessage);
    std::cout << "[DEBUG] PRIVMSG - Sending formatted message: " << formattedMsg;
    target->send(formattedMsg);
}

void PrivmsgCommandHandler::sendChannelMessage(Client& sender, const std::string& channelName, const std::string& message) {
	Channel* channel = server.getChannel(channelName);
	if (!channel) {
		sender.send("403 " + sender.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	if (!channel->hasClient(&sender)) {
		sender.send("404 " + sender.getNickname() + " " + channelName + " :Cannot send to channel\r\n");
		return;
	}

	channel->broadcastMessage(formatMessage(sender, channelName, message), &sender);
}
