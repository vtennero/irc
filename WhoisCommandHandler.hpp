#ifndef WHOIS_COMMAND_HANDLER_HPP
#define WHOIS_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

class WhoisCommandHandler : public CommandHandler {
public:
    WhoisCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif
