#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;

class Channel {
private:
    string name;
    vector<Client*> clients;
    vector<Client*> operators;
    string topic;
    string mode;
    string key;

public:
    Channel() : name(""), topic(""), mode("") {}
    Channel(const string& channelName);

    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(const Client* client) const;
    void broadcastMessage(const string& message, const Client* exclude = NULL);

    const string& getName() const;
    void setTopic(const string& newTopic) { topic = newTopic; }
    const string& getTopic() const { return topic; }
    vector<Client*> getClients() const { return clients; }

    // Channel management
    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(const Client* client) const;

    // List methods
    size_t getUserCount() const;
    vector<Client*> getClientList() const;
    string getClientListString() const;
    void setKey(const string& newKey) { key = newKey; }
    bool checkKey(const string& attemptedKey) const { return key.empty() || key == attemptedKey; }

};

#endif // CHANNEL_HPP
