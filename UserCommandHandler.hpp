#ifndef USER_COMMAND_HANDLER_HPP
#define USER_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

class UserCommandHandler : public CommandHandler {
public:
    UserCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);
private:
    void sendWelcomeMessages(Client& client);
};

#endif // USERCOMMANDHANDLER_HPP