#include "PrivmsgCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include <iostream>

PrivmsgCommandHandler::PrivmsgCommandHandler(Server& server) : CommandHandler(server) {}

// Validation helpers
bool PrivmsgCommandHandler::validateMessageParams(const Message& message) {
    if (message.getParams().size() < 2) {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Not enough parameters" << endl;
        return false;
    }
    return true;
}

string PrivmsgCommandHandler::trimMessage(const string& message) {
    string trimmedMessage = message;
    size_t endpos = trimmedMessage.find_last_not_of(" \r\n");
    if (endpos != string::npos) {
        trimmedMessage = trimmedMessage.substr(0, endpos + 1);
    } else {
        trimmedMessage.clear();
    }
    return trimmedMessage;
}

bool PrivmsgCommandHandler::validateTarget(Client& sender, Client* target, const string& targetNick) {
    if (!target) {
        sender.send("401 " + sender.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return false;
    }

    if (!target->isRegistered()) {
        sender.send("401 " + sender.getNickname() + " " + targetNick + " :User not registered\r\n");
        return false;
    }
    return true;
}

// Logging helpers
void PrivmsgCommandHandler::logMessageAttempt(const string& senderNick, const string& target, const string& content) {
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Attempting to send from " << senderNick << " to " << target << endl;
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Message content: " << content << endl;
}

void PrivmsgCommandHandler::logFormattedMessage(const string& formattedMessage) {
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Sending formatted message: " << formattedMessage << endl;
}

// Message formatting
string PrivmsgCommandHandler::formatMessage(const Client& sender, const string& target, const string& message) {
    return ":" + sender.getNickname() +
           "!" + sender.getUsername() +
           "@" + sender.getHostname() +
           " PRIVMSG " + target +
           " :" + message + "\r\n";
}

string PrivmsgCommandHandler::formatBasicPrivmsg(const Client& sender, const string& target, const string& content) {
    string username = sender.getUsername().empty() ? "vitenner" : sender.getUsername();
    return ":" + sender.getNickname() +
           "!vitenner@" +
           sender.getHostname() +
           " PRIVMSG " + target +
           " :" + content;
}

// Main handlers
void PrivmsgCommandHandler::handle(Client& sender, const Message& message) {
    if (!validateMessageParams(message)) {
        return;
    }

    string target = message.getParams()[0];
    string content = message.getParams()[1];

    logMessageAttempt(sender.getNickname(), target, content);

    if (target[0] == '#') {
		sendChannelMessage(sender, target, content);
		return;
	}

    Client* targetClient = server.findClientByNickname(target);
    if (targetClient) {
        string formattedMessage = formatBasicPrivmsg(sender, target, content);
        logFormattedMessage(formattedMessage);
        targetClient->send(formattedMessage);
    } else {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " PRIVMSG - Target " << target << " not found" << endl;
        sender.send("401 " + sender.getNickname() + " " + target + " :No such nick/channel");
    }
}

void PrivmsgCommandHandler::sendPrivateMessage(Client& sender, const string& targetNick, const string& message) {
    string trimmedMessage = trimMessage(message);

    if (trimmedMessage.empty()) {
        sender.send("412 " + sender.getNickname() + " :No text to send\r\n");
        return;
    }

    logMessageAttempt(sender.getNickname(), targetNick, trimmedMessage);

    Client* target = server.findClientByNickname(targetNick);
    if (!validateTarget(sender, target, targetNick)) {
        return;
    }

    string formattedMsg = formatMessage(sender, targetNick, trimmedMessage);
    logFormattedMessage(formattedMsg);
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
