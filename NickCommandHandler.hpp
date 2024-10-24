#ifndef NICK_COMMAND_HANDLER_HPP
#define NICK_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include <map>

class NickCommandHandler : public CommandHandler {
public:
    NickCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif // NICKCOMMANDHANDLER_HPP