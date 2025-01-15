#include "PrivmsgCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include <iostream>


void PrivmsgCommandHandler::handle(Client& sender, const Message& message) {
	if (message.getParams().size() < 2) {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Not enough parameters" << endl;

		return;
	}

	string target = message.getParams()[0];
	string content = message.getParams()[1];
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Attempting to send from " << sender.getNickname() << " to " << target << endl;
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Message content: " << content << endl;

	//Jeff's entry
	//check if target is a channel
	if (target[0] == '#') {
		sendChannelMessage(sender, target, content);
		return;
	}
	// Find the target client
	Client* targetClient = server.findClientByNickname(target);
	if (targetClient) {
		// Keep username as 'vitenner' since that's what was used in registration
		string username = sender.getUsername().empty() ? "vitenner" : sender.getUsername();

		// Format strictly according to IRC protocol
		// :nick!user@host PRIVMSG target :message
		string formattedMessage = ":" + sender.getNickname() +
									 "!vitenner@" +  // Keep consistent with welcome message
									 sender.getHostname() +
									 " PRIVMSG " + target +
									 " :" + content;
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Sending formatted message: " << formattedMessage << endl;

		targetClient->send(formattedMessage);
	} else {
		cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Target " << target << " not found" << endl;

		sender.send("401 " + sender.getNickname() + " " + target + " :No such nick/channel");
	}
}


string PrivmsgCommandHandler::formatMessage(const Client& sender, const string& target, const string& message) {
	// Ensure proper IRC message format with \r\n
	return ":" + sender.getNickname() +
		   "!" + sender.getUsername() +
		   "@" + sender.getHostname() +
		   " PRIVMSG " + target +
		   " :" + message + "\r\n";
}

void PrivmsgCommandHandler::sendPrivateMessage(Client& sender, const string& targetNick, const string& message) {
	// Remove trailing whitespace from message using C++98 methods
	string trimmedMessage = message;
	size_t endpos = trimmedMessage.find_last_not_of(" \r\n");
	if (endpos != string::npos) {
		trimmedMessage = trimmedMessage.substr(0, endpos + 1);
	} else {
		trimmedMessage.clear();
	}

	// Rest of the validation
	if (trimmedMessage.empty()) {
		sender.send("412 " + sender.getNickname() + " :No text to send\r\n");
		return;
	}
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Attempting to send from " << sender.getNickname() << " to " << targetNick << endl;
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Message content: " << trimmedMessage << endl;


	Client* target = server.findClientByNickname(targetNick);
	if (!target) {
		sender.send("401 " + sender.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	if (!target->isRegistered()) {
		sender.send("401 " + sender.getNickname() + " " + targetNick + " :User not registered\r\n");
		return;
	}

	string formattedMsg = formatMessage(sender, targetNick, trimmedMessage);
	cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Sending formatted message: " << formattedMsg << endl;

	target->send(formattedMsg);
}

void PrivmsgCommandHandler::sendChannelMessage(Client& sender, const string& channelName, const string& message) {
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
