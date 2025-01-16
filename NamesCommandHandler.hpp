#ifndef NAMES_COMMAND_HANDLER_HPP
#define NAMES_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Channel.hpp"
# include "Debug.hpp"

class NamesCommandHandler : public CommandHandler {
public:
    NamesCommandHandler(Server& server);
    virtual void handle(Client& client, const Message& message);

private:
    vector<string> parseChannels(const string& channelList);
    void sendNames(Client& client, const string& channelName);
};

#endif
