#ifndef LIST_COMMAND_HANDLER_HPP
#define LIST_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Channel.hpp"
# include "Debug.hpp"

class ListCommandHandler : public CommandHandler {
public:
    ListCommandHandler(Server& server);
    virtual void handle(Client& client, const Message& message);

private:
    void sendChannelList(Client& client, const vector<string>& channels);
    vector<string> parseChannels(const string& channelList);
};

#endif
