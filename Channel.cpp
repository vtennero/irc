#include "Channel.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel(const string& channelName) : name(channelName), topic(""), key("") {
	mode['i'] = 0; //invite mode
	mode['t'] = 0; //topic mode
	mode['k'] = 0; //channel key
	mode['l'] = 0; //user limit, 50 by default
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
	/*if (clients.empty()) {
		return;
	}*/
	for (vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (exclude && *it == exclude)
    		continue;
		/*if (*it != exclude) {
			(*it)->send(message);
		}*/
		(*it)->send(message);
	}
}

void Channel::setTopic(const string& newTopic) {
	cout << "Setting new topic: " << newTopic << endl;
	topic = newTopic; 
	cout << "Topic is now: " << topic << endl;
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

void Channel::removeOperator(Client* client) {
	if (isOperator(client)) {
			operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
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

void Channel::setInvite(int option) { mode['i'] = option;}

bool Channel::checkMode(const char& mode) const {
	map<char, int>::const_iterator it = this->mode.find(mode);
	if (it != this->mode.end()) {
		cout << mode << ": " << it->second << endl;
		return it->second;
	}
	return false;
}

void Channel::addInvite(Client* target) {
	if (find(invited.begin(), invited.end(), target) == invited.end()) {
		invited.push_back(target);
	}
}

void Channel::setMode(const char key, int option) {
	cout << "changing mode" << endl;
	mode[key] = option;
}

bool Channel::isInvited(Client const* client) const {
	return find(invited.begin(), invited.end(), client) != invited.end();
}
