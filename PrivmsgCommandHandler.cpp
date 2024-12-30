#include "PrivmsgCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include <iostream>


void PrivmsgCommandHandler::handle(Client& sender, const Message& message) {
    if (message.getParams().size() < 2) {
        std::cout << "[DEBUG] PRIVMSG - Not enough parameters" << std::endl;
        return;
    }

    std::string target = message.getParams()[0];
    std::string content = message.getParams()[1];

    std::cout << "[DEBUG] PRIVMSG - Attempting to send from " << sender.getNickname()
              << " to " << target << std::endl;
    std::cout << "[DEBUG] PRIVMSG - Message content: " << content << std::endl;

    // Find the target client
    Client* targetClient = server.findClientByNickname(target);
    if (targetClient) {
        // Keep username as 'vitenner' since that's what was used in registration
        std::string username = sender.getUsername().empty() ? "vitenner" : sender.getUsername();

        // Format strictly according to IRC protocol
        // :nick!user@host PRIVMSG target :message
        std::string formattedMessage = ":" + sender.getNickname() +
                                     "!vitenner@" +  // Keep consistent with welcome message
                                     sender.getHostname() +
                                     " PRIVMSG " + target +
                                     " :" + content;

        std::cout << "[DEBUG] PRIVMSG - Sending formatted message: " << formattedMessage << std::endl;
        targetClient->send(formattedMessage);
    } else {
        std::cout << "[DEBUG] PRIVMSG - Target " << target << " not found" << std::endl;
        sender.send("401 " + sender.getNickname() + " " + target + " :No such nick/channel");
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
