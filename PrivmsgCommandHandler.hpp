#ifndef PRIVMSG_COMMAND_HANDLER_HPP
#define PRIVMSG_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class PrivmsgCommandHandler : public CommandHandler {
public:
    PrivmsgCommandHandler(Server& server);
    virtual void handle(Client& client, const Message& message);

private:
    void sendPrivateMessage(Client& sender, const string& targetNick, const string& message);
    void sendChannelMessage(Client& sender, const string& channelName, const string& message);
    string formatMessage(const Client& sender, const string& target, const string& message);
};

#endif // PRIVMSG_COMMAND_HANDLER_HPP
