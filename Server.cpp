#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include "NickCommandHandler.hpp"
#include "UserCommandHandler.hpp"
#include "QuitCommandHandler.hpp"
#include "PrivmsgCommandHandler.hpp"
#include "ModeCommandHandler.hpp"
#include "WhoisCommandHandler.hpp"
#include "PingCommandHandler.hpp"
#include "PongCommandHandler.hpp"
#include "JoinCommandHandler.hpp"
#include "PartCommandHandler.hpp"
#include "ListCommandHandler.hpp"
#include "NamesCommandHandler.hpp"

#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>

#include <stdio.h>

// privmsg
void Server::sendToClient(const string& nickname, const string& message) {
	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second.getNickname() == nickname) {
			it->second.send(message);
			return;
		}
	}
}

void Server::broadcastToChannel(const string& channelName, const string& message, const Client* exclude) {
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()) {
		it->second.broadcastMessage(message, exclude);
	}
}

Client* Server::findClientByNickname(const string& nickname) {
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Looking for: " << nickname << endl;
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Current clients in server:" << endl;

	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		cout << "  FD: " << it->first
				  << " Nick: " << it->second.getNickname()
				  << " Registered: " << (it->second.isRegistered() ? "Yes" : "No")
				  << endl;

		if (it->second.getNickname() == nickname) {
			cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Found matching client!" << endl;
			return &(it->second);
		}
	}

	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " No matching client found" << endl;
	return NULL;
}

bool Server::isChannelExist(const string& channelName) const {
	return channels.find(channelName) != channels.end();
}

Channel* Server::getChannel(const string& channelName) {
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()) {
		return &(it->second);
	}
	return NULL;
}

// Replace createChannel implementation
Channel* Server::createChannel(const string& channelName)
{
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()) {
		return &(it->second);
	}
	channels.insert(std::pair<string, Channel>(channelName, Channel(channelName)));
	return &(channels.find(channelName)->second);
}

// Helper function for find_if
struct FdComparer
{
	FdComparer(int fd) : fd_to_find(fd)
	{}
	bool operator()(const pollfd& pfd) const
	{
		return pfd.fd == fd_to_find;
	}
	int fd_to_find;
};

void Server::setNonBlocking(int fd)
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		cerr << "Error setting socket to non-blocking: " << strerror(errno) << endl;
	}
}

Server::Server(int port, const string& password) : serverSocket(-1), serverPassword(password)
{
    cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        cerr << "Error creating socket: " << strerror(errno) << endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << "Error binding socket: " << strerror(errno) << endl;
        close(serverSocket);
        serverSocket = -1;  // Mark as invalid
        return;
    }

    if (listen(serverSocket, 5) == -1)
    {
        cerr << "Error listening on socket: " << strerror(errno) << endl;
        close(serverSocket);
        serverSocket = -1;  // Mark as invalid
        return;
    }

    setNonBlocking(serverSocket);

	commandHandlers["NICK"] = new NickCommandHandler(*this);
	commandHandlers["USER"] = new UserCommandHandler(*this);
	commandHandlers["QUIT"] = new QuitCommandHandler(*this);
	commandHandlers["PRIVMSG"] = new PrivmsgCommandHandler(*this);
	commandHandlers["MODE"] = new ModeCommandHandler(*this);
	commandHandlers["WHOIS"] = new WhoisCommandHandler(*this);
	commandHandlers["PING"] = new PingCommandHandler(*this);
	commandHandlers["PONG"] = new PongCommandHandler(*this);
	commandHandlers["JOIN"] = new JoinCommandHandler(*this);
	commandHandlers["PART"] = new PartCommandHandler(*this);
	commandHandlers["LIST"] = new ListCommandHandler(*this);
	commandHandlers["NAMES"] = new NamesCommandHandler(*this);

	cout << "Server listening on port " << port << endl;
}

Server::~Server()
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	if (serverSocket != -1)
	{
		close(serverSocket);
	}
	for (size_t i = 0; i < clientFds.size(); ++i)
	{
		close(clientFds[i].fd);
	}

	map<string, CommandHandler*>::iterator it;
	for (it = commandHandlers.begin(); it != commandHandlers.end(); ++it)
	{
		delete it->second;
	}
}


void Server::run()
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	pollfd serverPollFd;
	serverPollFd.fd = serverSocket;
	serverPollFd.events = POLLIN;
	clientFds.push_back(serverPollFd);

	while (true)
	{
		// Add ping check every iteration
		checkClientPings();

		// Update poll events for clients with pending data
		for (size_t i = 1; i < clientFds.size(); ++i) // Skip server socket
		{
			Client& client = clients[clientFds[i].fd];
			clientFds[i].events = POLLIN;
			if (client.hasDataToSend()) {
				cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Client fd " << clientFds[i].fd << " has pending data, adding POLLOUT" << endl;
				clientFds[i].events |= POLLOUT;
			}
		}

		int pollResult = poll(&clientFds[0], clientFds.size(), 1000); // 1 second timeout
		if (pollResult == -1)
		{
			cerr << "Error in poll" << endl;
			break;
		}
		// cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Poll returned " << pollResult << " events" << endl;


		for (size_t i = 0; i < clientFds.size(); ++i)
		{
			// cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Checking fd " << clientFds[i].fd << endl;


			if (clientFds[i].revents & POLLIN)
			{
				if (clientFds[i].fd == serverSocket)
				{
					handleNewConnection();
				}
				else
				{
					handleClientData(i);
				}
			}
			if (clientFds[i].revents & POLLOUT)
			{
				cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " POLLOUT event for client fd " << clientFds[i].fd << endl;
				// Try to send pending data
				clients[clientFds[i].fd].tryFlushSendBuffer();
			}
		}
	}
}

void Server::handleNewConnection()
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientSocket == -1)
	{
		cerr << "Error accepting client connection: " << strerror(errno) << endl;
		return;
	}
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Accepted new connection on socket " << clientSocket << endl;

	// Set non-blocking mode
	setNonBlocking(clientSocket);

	// Set smaller send buffer to help test non-blocking behavior
	int sndBufSize = 4096; // 4KB send buffer
	if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, &sndBufSize, sizeof(sndBufSize)) == -1) {
		cerr << "Warning: Could not set send buffer size: " << strerror(errno) << endl;
	}

	char hostBuffer[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &(clientAddr.sin_addr), hostBuffer, INET_ADDRSTRLEN) == NULL)
	{
		cerr << "Error converting client address to string: " << strerror(errno) << endl;
		close(clientSocket);
		return;
	}

	// Create client and add to poll list
	clients[clientSocket] = Client(clientSocket, string(hostBuffer));

	pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clientFds.push_back(clientPollFd);

	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " New client connected: " << hostBuffer << endl;

}


void Server::handleClientData(size_t index)
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << clientFds[index].fd << endl;
	char buffer[1024];
	int clientFd = clientFds[index].fd;
	int bytesRead = recv(clientFds[index].fd, buffer, sizeof(buffer), 0);


	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " fd " << clientFds[index].fd << " got " << bytesRead << " bytes:" << endl;
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Raw hex: ";
	for (int i = 0; i < bytesRead; i++) {
		printf("%02x ", (unsigned char)buffer[i]);
	}
	cout << endl;
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " ASCII: ";
	for (int i = 0; i < bytesRead; i++) {
		if (isprint(buffer[i])) cout << buffer[i];
		else cout << '.';
	}
	cout << endl;
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Received bytes: " << bytesRead << endl;
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Raw data: [" << endl;

	for (int i = 0; i < bytesRead; i++)
	{
		if (buffer[i] == '\r') cout << "\\r";
		else if (buffer[i] == '\n') cout << "\\n";
		else cout << buffer[i];
	}
	cout << "]" << endl;

	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
		{
			cout << "Client disconnected" << endl;
		}
		else
		{
			cerr << "Error reading from client: " << strerror(errno) << endl;
		}
		close(clientFds[index].fd);
		clientFds.erase(clientFds.begin() + index);
	}
	else
	{
		clients[clientFd].appendToBuffer(string(buffer, bytesRead));

		// Process any complete messages
		vector<string> completeMessages = clients[clientFd].getCompleteMessages();
		for (size_t i = 0; i < completeMessages.size(); i++)
		{
			cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Processing message: " << completeMessages[i] << endl;

			// Parse the message first
			Message parsedMessage(completeMessages[i]);

			// Regular command processing
			if (parsedMessage.getCommand() == "PASS")
			{
				if (parsedMessage.getParams().empty())
				{
					// Replace raw send with client.send and proper error code
					clients[clientFd].send("461 " + clients[clientFd].getNickname() + " PASS :Not enough parameters\r\n");
					return;
				}
				string password = parsedMessage.getParams()[0];
				if (clients[clientFd].isAuthenticated())
				{
					cout << "Client already authenticated" << endl;
					// Add proper error response for already authenticated
					clients[clientFd].send("462 " + clients[clientFd].getNickname() + " :You may not reregister\r\n");
					return;
				}
				if (authenticateClient(password, clientFd))
				{
					// Success message using client.send
					clients[clientFd].send("001 " + clients[clientFd].getNickname() + " :Password accepted\r\n");
				}
				else
				{
					// Already using client.send - this is correct
					clients[clientFd].send("464 " + clients[clientFd].getNickname() + " :Password incorrect\r\n");
					close(clientFd);
					clientFds.erase(clientFds.begin() + index);
					clients.erase(clientFd);
				}
			}
			else if (!clients[clientFd].isAuthenticated())
			{
				// Replace raw send with client.send and proper error code
				clients[clientFd].send("464 " + clients[clientFd].getNickname() + " :Password required\r\n");
			}
			else
			{
				map<string, CommandHandler*>::iterator handler;
				// Use regular command handler
				handler = commandHandlers.find(parsedMessage.getCommand());
				if (handler != commandHandlers.end())
				{
					handler->second->handle(clients[clientFd], parsedMessage);
				}
				else
				{
					cout << "Unknown command: " << parsedMessage.getCommand() << endl;
				}
			}
		}
	}
}

bool Server::authenticateClient(const string& password, int clientFd)
{
    // cout << "[" << __PRETTY_FUNCTION__ <<"] called" << endl;
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
    if (clients[clientFd].isAuthenticated())
    {
        cout << "Client already authenticated" << endl;
        return true;
    }
    if (password == serverPassword)
    {
        clients[clientFd].setAuthenticated(true);
        return true;
    }
    return false;
}

string Server::getServerName()
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	return "YourIRCServer";
}

string Server::getVersion()
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	return "1.0";
}

string Server::getCreationDate()
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	return "May 1, 2023";
}

void Server::removeClient(int fd) {
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called for fd: " << fd << endl;


	// First, remove from channels to avoid dangling pointers
	for (map<string, Channel>::iterator chIt = channels.begin(); chIt != channels.end(); ++chIt) {
		chIt->second.removeClient(&clients[fd]);
	}

	// Close the socket first
	close(fd);

	// Remove from clients map
	map<int, Client>::iterator clientIt = clients.find(fd);
	if (clientIt != clients.end()) {
		clients.erase(clientIt);
	}

	// Remove from pollfd vector
	vector<pollfd>::iterator pollIt = find_if(clientFds.begin(), clientFds.end(), FdComparer(fd));
	if (pollIt != clientFds.end()) {
		clientFds.erase(pollIt);
	}
}

vector<Channel*> Server::getClientChannels(const Client& client)
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	vector<Channel*> clientChannels;
	for (map<string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (it->second.hasClient(&client))
		{
			clientChannels.push_back(&(it->second));
		}
	}
	return clientChannels;
}

void Server::broadcastMessage(const string& message, const Client* exclude)
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (&(it->second) != exclude)
		{
			it->second.send(message);
		}
	}
}

bool Server::isNicknameInUse(const string& nickname) const
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	map<int, Client>::const_iterator it;
	for (it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
		{
			return true;
		}
	}
	return false;
}

void Server::checkClientPings() {
	vector<int> timeoutFds; // Store FDs to remove

	// First identify clients to remove
	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second.isPingTimedOut()) {
			cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Client " << it->first << " ping timed out, queueing for disconnect" << endl;

			timeoutFds.push_back(it->first);
		} else if (it->second.needsPing()) {
			cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Sending ping to client " << it->first << endl;

			it->second.sendPing();
		}
	}

	// Then remove them separately to avoid iterator invalidation
	for (vector<int>::iterator it = timeoutFds.begin(); it != timeoutFds.end(); ++it) {
		cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Removing timed out client fd: " << *it << endl;

		removeClient(*it);
	}
}
