#ifndef WHOIS_COMMAND_HANDLER_HPP
#define WHOIS_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class WhoisCommandHandler : public CommandHandler {
public:
    WhoisCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif
