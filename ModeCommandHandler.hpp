#ifndef MODE_COMMAND_HANDLER_HPP
#define MODE_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

class ModeCommandHandler : public CommandHandler {
public:
    ModeCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif
