#include "PongCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void PongCommandHandler::handle(Client& client, const Message& message) {
    if (message.getParams().empty()) {
        cout << SALMON "[" << __PRETTY_FUNCTION__ << "]" RESET " PONG received without token" << endl;
        return;
    }

    string token = message.getParams()[0];
    cout << SALMON "[" << __PRETTY_FUNCTION__ << "]" RESET " PONG received with token: " << token << endl;

    if (!client.verifyPongToken(token)) {
        cout << SALMON "[" << __PRETTY_FUNCTION__ << "]" RESET " PONG received with invalid token" << endl;
        // Could disconnect client here if needed
        return;
    }
}
