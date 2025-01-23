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
	vector<bool> clientGuestenticated;
	map<string, CommandHandler*> commandHandlers;
	map<int, Client> clients;
	map<string, Channel> channels;
    map<string, string> authednicks;

	void setNonBlocking(int socket);
	void handleNewConnection();
	void handleClientData(size_t index);
    void checkClientPings();  // Add ping check method declaration

    static Server* instance;  // Add this static member
    static void signalHandler(int signum);  // Add this static method
    void cleanup();  // Add this instance method

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

    int getSocket() const;

    vector<Channel*> getAllChannels();
    void setupSignalHandling();  // Changed to non-static

	void listAvailableCommands();

    bool addAuthNick(const string& nickname, const string& password);
    bool isNickAuthed(const string& nickname) const;
    string getAuthPassword(const string& nickname) const;
	bool guestenticateClient(const string& password, int clientFd);
};

#endif
