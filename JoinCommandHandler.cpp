#include "JoinCommandHandler.hpp"
#include "Server.hpp"
#include "Message.hpp"

JoinCommandHandler::JoinCommandHandler(Server& server) : CommandHandler(server) {}


void JoinCommandHandler::handle(Client& client, const Message& message) {
	if (message.getParams().empty()) {
		client.send("461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
		return;
	}

	vector<string> channels = parseChannels(message.getParams()[0]);
	vector<string> keys;
	if (message.getParams().size() > 1) {
		keys = parseKeys(message.getParams()[1]);
	}

	for (size_t i = 0; i < channels.size(); i++) {
		string channelName = channels[i];
		string key = (i < keys.size()) ? keys[i] : "";

		if (!isValidChannelName(channelName)) {
			client.send("403 " + client.getNickname() + " " + channelName + " :Invalid channel name\r\n");
			continue;
		}

		joinChannel(client, channelName, key);
	}
}

vector<string> JoinCommandHandler::parseChannels(const string& channelList) {
	vector<string> channels;
	size_t start = 0, end;

	while ((end = channelList.find(',', start)) != string::npos) {
		channels.push_back(channelList.substr(start, end - start));
		start = end + 1;
	}
	channels.push_back(channelList.substr(start));
	return channels;
}

vector<string> JoinCommandHandler::parseKeys(const string& keyList) {
	if (keyList.empty()) {
		return vector<string>();
	}

	vector<string> keys;
	size_t start = 0, end;

	while ((end = keyList.find(',', start)) != string::npos) {
		keys.push_back(keyList.substr(start, end - start));
		start = end + 1;
	}
	keys.push_back(keyList.substr(start));
	return keys;
}

bool JoinCommandHandler::isValidChannelName(const string& name) {
	return !name.empty() && name[0] == '#' && name.length() <= 50;
}

// Update joinChannel to use pointer return
void JoinCommandHandler::joinChannel(Client& client, const string& channelName, const string& key) {
	Channel* channel = server.getChannel(channelName);
	if (!channel) {
		// Create new channel
		channel = server.createChannel(channelName);
		if (!channel) {
			client.send("473 " + client.getNickname() + " " + channelName + " :Cannot join channel\r\n");
			return;
		}
		channel->addOperator(&client);
		if (!key.empty()) {
			channel->setKey(key);
		}
	} else if (!key.empty() && !channel->checkKey(key)) {
		client.send("475 " + client.getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n");
		return;
	}

	if (channel->hasClient(&client))
		return;

	// Add client to channel
	channel->addClient(&client);
	sendJoinMessages(client, *channel);
}

void JoinCommandHandler::sendJoinMessages(Client& client, Channel& channel) {
	string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" +
					client.getHostname() + " JOIN " + channel.getName() + "\r\n";

	channel.broadcastMessage(joinMsg);

	// Send topic if exists
	if (!channel.getTopic().empty()) {
		client.send("332 " + client.getNickname() + " " + channel.getName() +
				   " :" + channel.getTopic() + "\r\n");
	}

	// Send names list
	client.send("353 " + client.getNickname() + " = " + channel.getName() +
				" :" + channel.getClientListString() + "\r\n");
	client.send("366 " + client.getNickname() + " " + channel.getName() +
				" :End of /NAMES list.\r\n");
}
