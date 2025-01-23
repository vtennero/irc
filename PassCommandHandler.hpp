#ifndef PASS_COMMAND_HANDLER_HPP
#define PASS_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Debug.hpp"

class PassCommandHandler : public CommandHandler {
public:
    PassCommandHandler(Server& server);
    virtual void handle(Client& client, const Message& message);

private:
    bool validateMessageParameters(const Message& message, Client& client);
};

#endif
