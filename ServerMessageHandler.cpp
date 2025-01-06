#include "ServerMessageHandler.hpp"
#include <iostream>

void ServerMessageHandler::handle(Client& client, const Message& message) {
    cout << "[DEBUG] Processing server message in handler" << endl;
    cout << "[DEBUG] Raw message: " << message.getRawMessage() << endl;

    if (message.getCommand() == "PRIVMSG") {
        // For PRIVMSG, just forward the original message as-is
        // The formatting was already done by the PrivmsgCommandHandler
        client.send(message.getRawMessage());
    }
    // Add handling for other server messages if needed
}
