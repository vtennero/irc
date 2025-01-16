#include "NamesCommandHandler.hpp"
#include "Server.hpp"

NamesCommandHandler::NamesCommandHandler(Server& server) : CommandHandler(server) {}

void NamesCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		// List all channels
		vector<Channel*> allChannels = server.getAllChannels();
		for (vector<Channel*>::iterator it = allChannels.begin(); it != allChannels.end(); ++it) {
			sendNames(client, (*it)->getName());
		}
	} else {
		// List specific channels
		vector<string> channels = parseChannels(message.getParams()[0]);
		for (vector<string>::iterator it = channels.begin(); it != channels.end(); ++it) {
			sendNames(client, *it);
		}
	}

	client.send("366 " + client.getNickname() + " * :End of /NAMES list\r\n");
}

void NamesCommandHandler::sendNames(Client& client, const string& channelName) {
	Channel* channel = server.getChannel(channelName);
	if (!channel) {
		return;
	}

	string userList = channel->getClientListString();
	if (!userList.empty()) {
		client.send("353 " + client.getNickname() + " = " + channelName + " :" + userList + "\r\n");
	}
}

vector<string> NamesCommandHandler::parseChannels(const string& channelList) {
	vector<string> channels;
	size_t start = 0, end;

	while ((end = channelList.find(',', start)) != string::npos) {
		channels.push_back(channelList.substr(start, end - start));
		start = end + 1;
	}
	channels.push_back(channelList.substr(start));
	return channels;
}
