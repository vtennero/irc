#include "Channel.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel(const std::string& channelName) : name(channelName) {
	std::cout << "[DEBUG] function Channel constructor called with name: " << channelName << std::endl;
}

void Channel::addClient(Client* client) {
	std::cout << "[DEBUG] function addClient called with client fd: " << client->getFd() << std::endl;
	if (!hasClient(client)) {
		clients.push_back(client);
	}
}

void Channel::removeClient(Client* client) {
	std::cout << "[DEBUG] function removeClient called with client fd: " << client->getFd() << std::endl;
	clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}

bool Channel::hasClient(const Client* client) const {
	std::cout << "[DEBUG] function hasClient called with client fd: " << client->getFd() << std::endl;
	return std::find(clients.begin(), clients.end(), client) != clients.end();
}

void Channel::broadcastMessage(const std::string& message, const Client* exclude) {
	std::cout << "[DEBUG] function broadcastMessage called with message length: " << message.length() << std::endl;
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (*it != exclude) {
			(*it)->send(message);
		}
	}
}

const std::string& Channel::getName() const {
	std::cout << "[DEBUG] function getName called" << std::endl;
	return name;
}
