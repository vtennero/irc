// ServerMessageHandler.hpp
#ifndef SERVERMESSAGEHANDLER_HPP
#define SERVERMESSAGEHANDLER_HPP

#include "CommandHandler.hpp"
#include "Client.hpp"
#include "Message.hpp"

class ServerMessageHandler : public CommandHandler {
public:
    ServerMessageHandler(Server& server) : CommandHandler(server) {}
    void handle(Client& client, const Message& message);
};

#endif
