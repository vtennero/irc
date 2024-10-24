#include "UserCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include <iostream>
#include <sstream>

void UserCommandHandler::handle(Client& client, const Message& message) {
    if (client.isRegistered()) {
        client.send("462 " + client.getNickname() + " :You may not reregister\r\n");
        return;
    }

    if (message.getParams().size() < 4) {
        client.send("461 " + client.getNickname() + " USER :Not enough parameters\r\n");
        return;
    }
    
    client.setUsername(message.getParams()[0]);
    client.setRealname(message.getParams()[3]);
    std::cout << "Client set username to: " << client.getUsername() << std::endl;
    
    if (client.isAuthenticated() && !client.getNickname().empty()) {
        client.setRegistered(true);
        sendWelcomeMessages(client);
    }
}

void UserCommandHandler::sendWelcomeMessages(Client& client) {
    std::string nickname = client.getNickname();
    std::string username = client.getUsername();
    std::string hostname = client.getHostname();

    // 001 RPL_WELCOME
    client.send("001 " + nickname + " :Welcome to the Internet Relay Network " 
                + nickname + "!" + username + "@" + hostname + "\r\n");

    // 002 RPL_YOURHOST
    client.send("002 " + nickname + " :Your host is " + server.getServerName() 
                + ", running version " + server.getVersion() + "\r\n");

    // 003 RPL_CREATED
    client.send("003 " + nickname + " :This server was created " 
                + server.getCreationDate() + "\r\n");

    // 004 RPL_MYINFO
    client.send("004 " + nickname + " " + server.getServerName() + " " 
                + server.getVersion() + " <available user modes> <available channel modes>\r\n");
}