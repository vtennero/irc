#include "Channel.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel(const string& channelName) : name(channelName), topic(""), key("")
{
	mode["i"] = 0; //invite mode
	mode["t"] = 0; //topic mode
	mode["k"] = 0; //channel key
	mode["l"] = 50; //user limit, 50 by default
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
	if (isOperator(client)) {
		removeOperator(client);
	}
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

void Channel::broadcastMessageOps(const string& message, const Client* exclude) {
	cout << YELLOW "[" << __PRETTY_FUNCTION__ << "]" RESET " called with message length: " << message.length() << endl;

	for (vector<Client*>::const_iterator it = operators.begin(); it != operators.end(); ++it) {
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

void Channel::setInvite(int option) { mode["i"] = option;}

bool Channel::checkMode(const string& mode) const {
	map<string, int>::const_iterator it = this->mode.find(mode);
	if (it != this->mode.end()) {
		return it->second;
	}
	return false;
}
