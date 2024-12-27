#include "PingCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void PingCommandHandler::handle(Client& client, const Message& message) {
    std::cout << "[DEBUG] PING command received" << std::endl;

    // IRC PING-PONG format:
    // Client sends: PING <token>
    // Server responds: PONG <server> <token>

    std::string token = message.getParams().empty() ? server.getServerName() : message.getParams()[0];
    std::string response = "PONG " + server.getServerName() + " :" + token + "\r\n";

    std::cout << "[DEBUG] Sending PONG response: " << response;
    client.send(response);
}
