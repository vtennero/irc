#ifndef PART_COMMAND_HANDLER_HPP
#define PART_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Channel.hpp"

class PartCommandHandler : public CommandHandler {
public:
    PartCommandHandler(Server& server) : CommandHandler(server) {}
    virtual void handle(Client& client, const Message& message);

private:
    vector<string> parseChannels(const string& channelList);
    void partChannel(Client& client, const string& channelName, const string& reason);
};

#endif
