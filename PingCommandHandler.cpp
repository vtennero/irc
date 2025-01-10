#include "PingCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void PingCommandHandler::handle(Client& client, const Message& message) {
    cout << SALMON "[" << __PRETTY_FUNCTION__ << "]" RESET " PING command received with token: " << (message.getParams().empty() ? "none" : message.getParams()[0]) << endl;


    string token = message.getParams().empty() ?
                       server.getServerName() :
                       message.getParams()[0];

    // Simple PONG response format: "PONG :token"
    string response = "PONG :" + token + "\r\n";

    cout << SALMON "[" << __PRETTY_FUNCTION__ << "]" RESET " Sending PONG response: " << response << endl;

    client.send(response);
}
