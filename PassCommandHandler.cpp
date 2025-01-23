#include "PassCommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

PassCommandHandler::PassCommandHandler(Server& server) : CommandHandler(server) {}

bool PassCommandHandler::validateMessageParameters(const Message& message, Client& client) {
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Validating PASS parameters" << endl;

    // Check if already registered
    if (client.isRegistered()) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Client already registered" << endl;
        client.send("462 " + client.getNickname() + " :You may not reregister\r\n");
        return false;
    }

    // Check if already guestenticated
    if (client.isGuestenticated()) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Client already authenticated" << endl;
        client.send("462 " + client.getNickname() + " :Already authenticated\r\n");
        return false;
    }

    // Check for empty params - moved to handle()
    if (message.getParams().empty()) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " No password provided" << endl;
        client.send("461 * PASS :Not enough parameters\r\n");
        return false;
    }

    // Now safe to check password
    string password = message.getParams()[0];
    if (password.empty()) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Empty password" << endl;
        client.send("461 * PASS :Password cannot be empty\r\n");
        return false;
    }

    cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Parameters validation successful" << endl;
    return true;
}

void PassCommandHandler::handle(Client& client, const Message& message) {
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Processing PASS command" << endl;

    // First validate parameters exist
    if (message.getParams().empty()) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " No parameters provided" << endl;
        client.send("461 * PASS :Not enough parameters\r\n");
        return;
    }

    // Validate message parameters
    if (!validateMessageParameters(message, client)) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Parameter validation failed" << endl;
        return;
    }

    string password = message.getParams()[0];
    cout << ORANGE "[" << __PRETTY_FUNCTION__ << "]" RESET " Attempting authentication with provided password" << endl;

    // Try to authenticate
    try {
        if (server.guestenticateClient(password, client.getFd())) {
            cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Authentication successful" << endl;
            client.send(":" + server.getServerName() + " NOTICE * :Password accepted\r\n");
            return;
        }

        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Authentication failed - wrong password" << endl;
        client.send("464 * :Password incorrect\r\n");

    } catch (const std::exception& e) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error during authentication: " << e.what() << endl;
        client.send("465 * :Internal server error\r\n");
    }
}
