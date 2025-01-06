// ServerMessageHandler.hpp
#ifndef SERVERMESSAGEHANDLER_HPP
#define SERVERMESSAGEHANDLER_HPP

#include "CommandHandler.hpp"
#include "Client.hpp"
#include "Message.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class ServerMessageHandler : public CommandHandler {
public:
    ServerMessageHandler(Server& server) : CommandHandler(server) {}
    void handle(Client& client, const Message& message);
};

#endif
