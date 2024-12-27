#ifndef PONG_COMMAND_HANDLER_HPP
#define PONG_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

class PongCommandHandler : public CommandHandler {
public:
    PongCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif
