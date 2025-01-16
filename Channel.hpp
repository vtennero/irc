#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"
# include "Debug.hpp"

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
	void setTopic(const string& newTopic);
	const string& getTopic() const;
	vector<Client*> getClients() const;
	void setKey(const string& newKey);
	bool checkKey(const string& attemptedKey) const;


	Channel();
	Channel(const string& channelName);

	void addClient(Client* client);
	void removeClient(Client* client);
	bool hasClient(const Client* client) const;
	void broadcastMessage(const string& message, const Client* exclude = NULL);

	const string& getName() const;

	// Channel management
	void addOperator(Client* client);
	void removeOperator(Client* client);
	bool isOperator(const Client* client) const;

	// List methods
	size_t getUserCount() const;
	vector<Client*> getClientList() const;
	string getClientListString() const;

};

#endif // CHANNEL_HPP
