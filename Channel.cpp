#include "Channel.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel(const string& channelName) : name(channelName) {
	cout << "[DEBUG] function Channel constructor called with name: " << channelName << endl;
}

void Channel::addClient(Client* client) {
	cout << "[DEBUG] function addClient called with client fd: " << client->getFd() << endl;
	if (!hasClient(client)) {
		clients.push_back(client);
	}
}

void Channel::removeClient(Client* client) {
	cout << "[DEBUG] function removeClient called with client fd: " << client->getFd() << endl;
	clients.erase(remove(clients.begin(), clients.end(), client), clients.end());
}

bool Channel::hasClient(const Client* client) const {
	cout << "[DEBUG] function hasClient called with client fd: " << client->getFd() << endl;
	return find(clients.begin(), clients.end(), client) != clients.end();
}

void Channel::broadcastMessage(const string& message, const Client* exclude) {
	cout << "[DEBUG] function broadcastMessage called with message length: " << message.length() << endl;
	for (vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (*it != exclude) {
			(*it)->send(message);
		}
	}
}

const string& Channel::getName() const {
    return name;
}

void Channel::addOperator(Client* client) {
    if (find(operators.begin(), operators.end(), client) == operators.end()) {
        operators.push_back(client);
    }
}

string Channel::getClientListString() const {
    string list;
    for (vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (isOperator(*it)) {
            list += "@";
        }
        list += (*it)->getNickname() + " ";
    }
    return list;
}

bool Channel::isOperator(const Client* client) const {
    return find(operators.begin(), operators.end(), client) != operators.end();
}

size_t Channel::getUserCount() const { return clients.size(); }
