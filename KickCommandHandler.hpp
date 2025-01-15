#ifndef KICK_COMMAND_HANDLER_HPP
#define KICK_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Channel.hpp"
# include "Debug.hpp"

class KickCommandHandler : public CommandHandler {
public:
    KickCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif