#include "NickCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>

NickCommandHandler::NickCommandHandler(Server& server) : CommandHandler(server) {}
//upon changing nick, set authenticated to false

void NickCommandHandler::handle(Client& client, const Message& message) {
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << client.getFd() << endl;


    // to update
    // 1. check in the db if newnick exists and is connected, no, which we already do
    // 2. check if authed db, if yes then fetch password, then we ask the user for the password prompt, then we check if its equal to the password we just fetched.
    // OR we check its in the auth db, we just return true or false, then ask user for the password prompt, then go through the whole db again but we will duplicate erros if duplicate pwd
    // after nick is changed, auth status is lost unless it

    string newNick;
    if (!validateNicknameRequest(client, message, newNick)) {
        return;
    }

    handleNicknameChange(client, newNick);
}

bool NickCommandHandler::validateNicknameRequest(Client& client, const Message& message, string& newNick) {
    if (message.getParams().empty()) {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " No nickname provided" << endl;
        client.send("431 " + client.getNickname() + " :No nickname given\r\n");
        return false;
    }

    newNick = message.getParams()[0];
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Requested nickname: " << newNick << endl;

    if (newNick == "NickServ" || newNick == "nickserv") {
        client.send("432 " + newNick + " :Reserved nickname\r\n");
        return false;
    }

    if (!isValidNickname(newNick)) {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Invalid nickname format" << endl;
        client.send("432 " + newNick + " :Erroneous nickname\r\n");
        return false;
    }

    if (server.isNickAuthed(newNick)) {
        client.setAwaitAuth(true, time(NULL) + 10); // 10 second timeout
        client.send(":" + server.getServerName() + " NOTICE " + client.getNickname() +
                   " :This nickname is registered. Please authenticate within 10 seconds using:\r\n");
        client.send(":" + server.getServerName() + " NOTICE " + client.getNickname() +
                   " :/msg NickServ identify <password>\r\n");
    }

    if (server.isNicknameInUse(newNick)) {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Nickname already in use" << endl;
        client.send("433 " + newNick + " :Nickname is already in use\r\n");
        return false;
    }

    return true;
}

void NickCommandHandler::handleNicknameChange(Client& client, const string& newNick) {

    string oldNick = client.getNickname();
    client.setNickname(newNick);
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Nickname changed from '"
         << oldNick << "' to '" << newNick << "'" << endl;

    sendNickChangeNotification(client, oldNick, newNick);
    checkAndHandleRegistration(client);
}

void NickCommandHandler::sendNickChangeNotification(Client& client, const string& oldNick, const string& newNick) {
    string nickMsg = ":" + oldNick + "!" + client.getUsername() + "@" +
                    client.getHostname() + " NICK :" + newNick + "\r\n";
    client.send(nickMsg);
}

void NickCommandHandler::checkAndHandleRegistration(Client& client) {
    if (!client.isRegistered() && client.isGuestenticated() && !client.getUsername().empty()) {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET
             << " Client fully registered, sending welcome message" << endl;

        client.setRegistered(true);
        client.send("001 " + client.getNickname() + " :Welcome to the Internet Relay Network " +
                   client.getNickname() + "!" + client.getUsername() + "@" +
                   client.getHostname() + "\r\n");
    }
}

bool NickCommandHandler::isValidNickname(const string& nick) {
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with nickname: " << nick << endl;
    return validateNicknameFormat(nick);
}

bool NickCommandHandler::validateNicknameFormat(const string& nick) {
    if (nick.empty() || nick.length() > 9) {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET
             << " Nickname length invalid: " << nick.length() << " chars" << endl;
        return false;
    }

    if (!validateFirstCharacter(nick[0])) {
        cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET
             << " Invalid first character: " << nick[0] << endl;
        return false;
    }

    for (size_t i = 1; i < nick.length(); i++) {
        if (!validateCharacter(nick[i])) {
            cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET
                 << " Invalid character at position " << i << ": " << nick[i] << endl;
            return false;
        }
    }

    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Nickname validation successful" << endl;
    return true;
}

bool NickCommandHandler::validateFirstCharacter(char c) {
    return isalpha(c) || c == '[' || c == ']' || c == '\\' || c == '`' ||
           c == '_' || c == '^' || c == '{' || c == '|';
}

bool NickCommandHandler::validateCharacter(char c) {
    return isalnum(c) || c == '-' || c == '_';
}
