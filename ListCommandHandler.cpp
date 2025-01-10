#include "ListCommandHandler.hpp"
#include "Server.hpp"
#include <sstream>

// Add toString utility function at top
string toString(size_t value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

void ListCommandHandler::handle(Client& client, const Message& message) {
	// Start of list
	client.send("321 " + client.getNickname() + " Channel :Users Name\r\n");

	vector<string> targetChannels;
	if (!message.getParams().empty()) {
		targetChannels = parseChannels(message.getParams()[0]);
	}

	sendChannelList(client, targetChannels);

	// End of list
	client.send("323 " + client.getNickname() + " :End of /LIST\r\n");
}

void ListCommandHandler::sendChannelList(Client& client, const vector<string>& channels) {
	vector<Channel*> allChannels = server.getAllChannels();

	for (vector<Channel*>::iterator it = allChannels.begin(); it != allChannels.end(); ++it) {
		Channel* channel = *it;
		string channelName = channel->getName();

		// Skip if specific channels requested and this one isn't in the list
		if (!channels.empty()) {
			bool found = false;
			for (vector<string>::const_iterator cit = channels.begin(); cit != channels.end(); ++cit) {
				if (*cit == channelName) {
					found = true;
					break;
				}
			}
			if (!found) continue;
		}

		string response = "322 " + client.getNickname() + " " +
						 channelName + " " +
						 toString(channel->getUserCount()) + " :" +
						 channel->getTopic() + "\r\n";

		client.send(response);
	}
}

vector<string> ListCommandHandler::parseChannels(const string& channelList) {
	vector<string> channels;
	size_t start = 0, end;

	while ((end = channelList.find(',', start)) != string::npos) {
		channels.push_back(channelList.substr(start, end - start));
		start = end + 1;
	}
	channels.push_back(channelList.substr(start));
	return channels;
}
