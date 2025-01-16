#include "Channel.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel() : name(""), topic(""), mode("") {}


void Channel::setTopic(const string& newTopic) {
    topic = newTopic;
}

const string& Channel::getTopic() const {
    return topic;
}

vector<Client*> Channel::getClients() const {
    return clients;
}

void Channel::setKey(const string& newKey) {
    key = newKey;
}

bool Channel::checkKey(const string& attemptedKey) const {
    return key.empty() || key == attemptedKey;
}

Channel::Channel(const string& channelName) : name(channelName)
{
	cout << YELLOW "[" << __PRETTY_FUNCTION__ << "]" RESET " called with name: " << channelName << endl;

}

void Channel::addClient(Client* client) {
	cout << YELLOW "[" << __PRETTY_FUNCTION__ << "]" RESET " called with client fd: " << client->getFd() << endl;

	if (!hasClient(client)) {
		clients.push_back(client);
	}
}

void Channel::removeClient(Client* client) {
	cout << YELLOW "[" << __PRETTY_FUNCTION__ << "]" RESET " called with client fd: " << client->getFd() << endl;

	clients.erase(remove(clients.begin(), clients.end(), client), clients.end());
}

bool Channel::hasClient(const Client* client) const {
	cout << YELLOW "[" << __PRETTY_FUNCTION__ << "]" RESET " called with client fd: " << client->getFd() << endl;

	return find(clients.begin(), clients.end(), client) != clients.end();
}

void Channel::broadcastMessage(const string& message, const Client* exclude) {
	cout << YELLOW "[" << __PRETTY_FUNCTION__ << "]" RESET " called with message length: " << message.length() << endl;

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
