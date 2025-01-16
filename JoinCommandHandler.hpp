#ifndef JOIN_COMMAND_HANDLER_HPP
#define JOIN_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include "Channel.hpp"
# include "Debug.hpp"

class JoinCommandHandler : public CommandHandler {
public:
    JoinCommandHandler(Server& server);
    virtual void handle(Client& client, const Message& message);


private:
    vector<string> parseChannels(const string& channelList);
    vector<string> parseKeys(const string& keyList);
    bool isValidChannelName(const string& name);
    void joinChannel(Client& client, const string& channel, const string& key);
    void sendJoinMessages(Client& client, Channel& channel);

};

#endif
