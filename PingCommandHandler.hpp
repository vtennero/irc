#ifndef PING_COMMAND_HANDLER_HPP
#define PING_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class PingCommandHandler : public CommandHandler {
public:
    PingCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif
