#ifndef PRIVMSG_COMMAND_HANDLER_HPP
#define PRIVMSG_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

class PrivmsgCommandHandler : public CommandHandler {
public:
    PrivmsgCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);

private:
    void sendPrivateMessage(Client& sender, const std::string& targetNick, const std::string& message);
    void sendChannelMessage(Client& sender, const std::string& channelName, const std::string& message);
    std::string formatMessage(const Client& sender, const std::string& target, const std::string& message);
};

#endif // PRIVMSG_COMMAND_HANDLER_HPP
