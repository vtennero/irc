#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <sys/poll.h>
#include <cstddef>
#include <string>
#include <iostream>
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include <map>
# include "Debug.hpp"

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;
using std::cerr;

class Server
{
private:
	int serverSocket;

	vector<pollfd> clientFds;
	static const int MAX_CLIENTS = 100;
	string serverPassword;
	vector<bool> clientAuthenticated;
	map<string, CommandHandler*> commandHandlers;
	map<int, Client> clients;
	map<string, Channel> channels;

	void setNonBlocking(int socket);
	void handleNewConnection();
	void handleClientData(size_t index);
	bool authenticateClient(const string& password, int clientFd);
    void checkClientPings();  // Add ping check method declaration

public:
	Server(int port, const string& password);
	~Server();
	void run();

	static string getServerName();
	static string getVersion();
	static string getCreationDate();

	void removeClient(int fd);
	vector<Channel*> getClientChannels(const Client& client);
	void broadcastMessage(const string& message, const Client* exclude = NULL);
	bool isNicknameInUse(const string& nickname) const;

	void sendToClient(const string& nickname, const string& message);
	void broadcastToChannel(const string& channelName, const string& message, const Client* exclude = NULL);

	Client* findClientByNickname(const string& nickname);
    bool isChannelExist(const string& channelName) const;
    Channel* getChannel(const string& channelName);
	Channel* createChannel(const string& channelName); // Remove inline implementation

    int getSocket() const { return serverSocket; }

    vector<Channel*> getAllChannels() {
        vector<Channel*> result;
        for (map<string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
            result.push_back(&(it->second));
        }
        return result;
    }
};

#endif
