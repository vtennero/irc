#include "PrivmsgCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include <iostream>

// static std::string trim(const std::string& str) {
//     size_t first = str.find_first_not_of(" \t");
//     if (first == std::string::npos) return "";
//     size_t last = str.find_last_not_of(" \t");
//     return str.substr(first, last - first + 1);
// }

void PrivmsgCommandHandler::handle(Client& client, const Message& message) {
	if (!client.isRegistered()) {
		client.send("451 " + client.getNickname() + " :You have not registered\r\n");
		return;
	}

	// Check parameters
	if (message.getParams().size() < 2) {
		client.send("461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	std::string target = message.getParams()[0];
	// Take only the actual message part without duplication
	std::string msgText = message.getParams()[1];
	size_t colonPos = msgText.find(':');
	if (colonPos != std::string::npos) {
		// Extract only the text after the colon
		msgText = msgText.substr(colonPos + 1);
	}

	// Handle empty target or message
	if (target.empty()) {
		client.send("411 " + client.getNickname() + " :No recipient given (PRIVMSG)\r\n");
		return;
	}
	if (msgText.empty()) {
		client.send("412 " + client.getNickname() + " :No text to send\r\n");
		return;
	}

	// Handle channel or private messages
	if (target[0] == '#') {
		sendChannelMessage(client, target, msgText);
	} else {
		sendPrivateMessage(client, target, msgText);
	}
}


std::string PrivmsgCommandHandler::formatMessage(const Client& sender, const std::string& target, const std::string& message) {
	return ":" + sender.getNickname() + "!" + sender.getUsername() + "@" +
		   sender.getHostname() + " PRIVMSG " + target + " :" + message + "\r\n";
}

void PrivmsgCommandHandler::sendPrivateMessage(Client& sender, const std::string& targetNick, const std::string& message) {
	Client* target = server.findClientByNickname(targetNick);
	if (!target) {
		sender.send("401 " + sender.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	target->send(formatMessage(sender, targetNick, message));
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
