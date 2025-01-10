#ifndef PONG_COMMAND_HANDLER_HPP
#define PONG_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class PongCommandHandler : public CommandHandler {
public:
    PongCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
};

#endif
