#ifndef AUTH_COMMAND_HANDLER_HPP
#define AUTH_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class AuthCommandHandler : public CommandHandler {
public:
    AuthCommandHandler(Server& server);
    virtual void handle(Client& client, const Message& message);
};

#endif