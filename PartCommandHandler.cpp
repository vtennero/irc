#include "PartCommandHandler.hpp"
#include "Server.hpp"

void PartCommandHandler::handle(Client& client, const Message& message) {
    if (message.getParams().empty()) {
        client.send("461 " + client.getNickname() + " PART :Not enough parameters\r\n");
        return;
    }

    vector<string> channels = parseChannels(message.getParams()[0]);
    string reason = message.getParams().size() > 1 ? message.getParams()[1] : "Leaving";

    for (size_t i = 0; i < channels.size(); ++i) {
        partChannel(client, channels[i], reason);
    }
}

vector<string> PartCommandHandler::parseChannels(const string& channelList) {
    vector<string> channels;
    size_t start = 0, end;

    while ((end = channelList.find(',', start)) != string::npos) {
        channels.push_back(channelList.substr(start, end - start));
        start = end + 1;
    }
    channels.push_back(channelList.substr(start));
    return channels;
}

void PartCommandHandler::partChannel(Client& client, const string& channelName, const string& reason) {
    Channel* channel = server.getChannel(channelName);
    if (!channel) {
        client.send("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    if (!channel->hasClient(&client)) {
        client.send("442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    string partMessage = ":" + client.getNickname() + "!" + client.getUsername() + "@" +
                        client.getHostname() + " PART " + channelName + " :" + reason + "\r\n";

    channel->broadcastMessage(partMessage);
    channel->removeClient(&client);
}
