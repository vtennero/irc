#include "PrivmsgCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>

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

static string toLowerCase(const string& cmd) {
    string result = cmd;
    for (size_t i = 0; i < cmd.size(); ++i) {
        result[i] = static_cast<char>(std::tolower(result[i]));
    }
    return result;
}

void PrivmsgCommandHandler::runAuth(Client& sender, const string& msg)
{
    //need to split cmd by space, and ensure only 2 tokens, else throw error. 1st must be register/auth/identify, 2nd must be password.
    vector<string> cmd;
    std::istringstream stream(msg);
    string word;
    while (stream >> word) {
        cmd.push_back(word);
    }
    if (cmd.size() != 2) {
        //sender.send("461 " + sender.getNickname() + " " + "REGISTER/IDENTIFY" + " :Wrong no. of parameters: <REGISTER/IDENTIFY> <password>\r\n");
    }
    if (toLowerCase(cmd[0]) == "register") {
        //check if nick is already registered
        if (server.isNickAuthed(sender.getNickname())) {
            sender.send("462 " + sender.getNickname() + " " + "REGISTER" + " :Nickname is already registered, run /msg nickserv identify <password> instead\r\n");
        } else {
            if (!server.addAuthNick(sender.getNickname(), cmd[1])) {
                sender.send("464 " + sender.getNickname() + " " + "REGISTER" + " :Failure in registering nickname, please try another nickname or password.\r\n");
            } else {
                sender.send(sender.getNickname() + " " + "REGISTER" + " Success! Please run /msg nickserv identify <password>\r\n");
            }
        }
        return;
        //if not registered, add to db w password
        //give success message
        //prompt to run auth command
    }
    if (toLowerCase(cmd[0]) == "identify" || toLowerCase(cmd[0]) == "auth") {
        if (!server.isNickAuthed(sender.getNickname())) {
            sender.send("432 " + sender.getNickname() + " " + "IDENTIFY" + " :The nickname is not registered yet, please try /msg nickserv register <password instead\r\n");
            return;
        } else {
            if (server.getAuthPassword(sender.getNickname()) != cmd[1]) {
                sender.send("464 " + sender.getNickname() + " " + "IDENTIFY" + " :Password incorrect.\r\n");
                return;
            } else {
                cout << "is user authenticated? " << sender.isAuthenticated() << endl;
                sender.setAuthenticated();
                cout << "is user authenticated? " << sender.isAuthenticated() << endl;
            }
        }
        //check if nick is already registered
        //if not registered, ask to run register command
        //if registered, obtain key to match
        //give success message, change bool to authenticated
    }
}

// Main handlers
void PrivmsgCommandHandler::handle(Client& sender, const Message& message) {
    if (!validateMessageParams(message)) {
        return;
    }
    cout << "printing [0] onwards " << message.getParams()[0] << " 1: " << message.getParams()[1] << endl;
    if (toLowerCase(message.getParams()[0]) == "nickserv") {
        cout << "running auth command" << endl;
        runAuth(sender, message.getParams()[1]);
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
