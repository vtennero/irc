#include "PongCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void PongCommandHandler::handle(Client& client, const Message& message) {
    if (message.getParams().empty()) {
        std::cout << "[DEBUG] PONG received without token" << std::endl;
        return;
    }

    std::string token = message.getParams()[0];
    std::cout << "[DEBUG] PONG received with token: " << token << std::endl;

    if (!client.verifyPongToken(token)) {
        std::cout << "[DEBUG] PONG received with invalid token" << std::endl;
        // Could disconnect client here if needed
        return;
    }
}
