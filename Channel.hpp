#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include "Client.hpp"
# include "Debug.hpp"

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;

class Channel {
private:
	string name;
	vector<Client*> clients;
	vector<Client*> operators;
	vector<Client*> invited;
	string topic;
	map<string, int> mode;
	string key;
	int usersLimit;

public:
	Channel() : name(""), topic(""), mode("") {}
	Channel(const string& channelName);

	void addClient(Client* client);
	void addInvite(Client* target);
	void removeClient(Client* client);
	bool hasClient(const Client* client) const;
	void broadcastMessage(const string& message, const Client* exclude = NULL);
	void broadcastMessageOps(const string& message, const Client* exclude = NULL);

	const string& getName() const;
	void setTopic(const string& newTopic) { topic = newTopic; }
	const string& getTopic() const { return topic; }
	vector<Client*> getClients() const { return clients; }

	// Channel management
	void addOperator(Client* client);
	void removeOperator(Client* client);
	void setUsersLimit(int limit) { usersLimit = limit; };
	bool isOperator(const Client* client) const;
	void setInvite(int option); 

	// List methods
	size_t getUserCount() const;
	int getUsersLimit()	const { return usersLimit; }
	bool checkMode(const string& mode) const;
	bool isInvited(const Client* client) const;
	vector<Client*> getClientList() const;
	string getClientListString() const;
	void setKey(const string& newKey) { key = newKey; }
	bool checkKey(const string& attemptedKey) const { return key.empty() || key == attemptedKey; }

};

#endif // CHANNEL_HPP
