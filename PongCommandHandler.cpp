#include "PongCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void PongCommandHandler::handle(Client& client, const Message& message) {
    // Simply log PONG messages for now
    std::cout << "[DEBUG] PONG received from " << client.getNickname() << std::endl;
    if (!message.getParams().empty()) {
        std::cout << "[DEBUG] PONG token: " << message.getParams()[0] << std::endl;
    }
}
