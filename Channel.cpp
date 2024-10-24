#include "Channel.hpp"
#include <algorithm>

Channel::Channel(const std::string& channelName) : name(channelName) {}

void Channel::addClient(Client* client) {
    if (!hasClient(client)) {
        clients.push_back(client);
    }
}

void Channel::removeClient(Client* client) {
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}

bool Channel::hasClient(const Client* client) const {
    return std::find(clients.begin(), clients.end(), client) != clients.end();
}

void Channel::broadcastMessage(const std::string& message, const Client* exclude) {
    for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it != exclude) {
            (*it)->send(message);
        }
    }
}

const std::string& Channel::getName() const {
    return name;
}