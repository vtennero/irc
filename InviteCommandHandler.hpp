#ifndef INVITE_COMMAND_HANDLER_HPP
#define INVITE_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Channel.hpp"
# include "Debug.hpp"

class InviteCommandHandler : public CommandHandler {
public:
    InviteCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif