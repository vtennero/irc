#ifndef PING_COMMAND_HANDLER_HPP
#define PING_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

class PingCommandHandler : public CommandHandler {
public:
    PingCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif
