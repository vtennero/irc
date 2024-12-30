#include "ServerMessageHandler.hpp"
#include <iostream>

void ServerMessageHandler::handle(Client& client, const Message& message) {
    std::cout << "[DEBUG] Processing server message in handler" << std::endl;
    std::cout << "[DEBUG] Raw message: " << message.getRawMessage() << std::endl;

    if (message.getCommand() == "PRIVMSG") {
        // For PRIVMSG, just forward the original message as-is
        // The formatting was already done by the PrivmsgCommandHandler
        client.send(message.getRawMessage());
    }
    // Add handling for other server messages if needed
}
