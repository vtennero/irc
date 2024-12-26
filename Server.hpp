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

using std::string;
using std::vector;
using std::map;
using std::cerr;
using std::cout;
using std::endl;

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

	void sendToClient(const std::string& nickname, const std::string& message);
	void broadcastToChannel(const std::string& channelName, const std::string& message, const Client* exclude = NULL);

	Client* findClientByNickname(const std::string& nickname);
    bool isChannelExist(const std::string& channelName) const;
    Channel* getChannel(const std::string& channelName);
};

#endif
