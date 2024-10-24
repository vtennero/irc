#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel {
private:
    std::string name;
    std::vector<Client*> clients;

public:
    Channel(const std::string& channelName);

    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(const Client* client) const;
    void broadcastMessage(const std::string& message, const Client* exclude = NULL);

    const std::string& getName() const;
};

#endif // CHANNEL_HPP