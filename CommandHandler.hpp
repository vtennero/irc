#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include "Client.hpp"
#include "Message.hpp"

class CommandHandler {
protected:
    Server& server;

public:
    CommandHandler(Server& server) : server(server) {}
    virtual ~CommandHandler() {}
    virtual void handle(Client& client, const Message& message) = 0;
};

#endif // COMMANDHANDLER_HPP