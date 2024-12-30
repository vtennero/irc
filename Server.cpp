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
#include "ServerMessageHandler.hpp"

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
void Server::sendToClient(const std::string& nickname, const std::string& message) {
	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second.getNickname() == nickname) {
			it->second.send(message);
			return;
		}
	}
}

void Server::broadcastToChannel(const std::string& channelName, const std::string& message, const Client* exclude) {
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()) {
		it->second.broadcastMessage(message, exclude);
	}
}

Client* Server::findClientByNickname(const std::string& nickname) {
	std::cout << "[DEBUG] Server::findClientByNickname - Looking for: " << nickname << std::endl;
	std::cout << "[DEBUG] Current clients in server:" << std::endl;

	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		std::cout << "  FD: " << it->first
				  << " Nick: " << it->second.getNickname()
				  << " Registered: " << (it->second.isRegistered() ? "Yes" : "No")
				  << std::endl;

		if (it->second.getNickname() == nickname) {
			std::cout << "[DEBUG] Found matching client!" << std::endl;
			return &(it->second);
		}
	}

	std::cout << "[DEBUG] No matching client found" << std::endl;
	return NULL;
}

bool Server::isChannelExist(const std::string& channelName) const {
	return channels.find(channelName) != channels.end();
}

Channel* Server::getChannel(const std::string& channelName) {
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()) {
		return &(it->second);
	}
	return NULL;
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
	cout << "[DEBUG] function setNonBlocking called" << endl;
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		cerr << "Error setting socket to non-blocking: " << strerror(errno) << endl;
	}
}

Server::Server(int port, const string& password) : serverSocket(-1), serverPassword(password)
{
	cout << "[DEBUG] function Server constructor called" << endl;
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
		return;
	}

	if (listen(serverSocket, 5) == -1)
	{
		cerr << "Error listening on socket: " << strerror(errno) << endl;
		close(serverSocket);
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
	// commandHandlers["SERVER_MESSAGE"] = new ServerMessageHandler(*this);

	cout << "Server listening on port " << port << endl;
}

Server::~Server()
{
	cout << "[DEBUG] function Server destructor called" << endl;
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

// old working version
// void Server::run()
// {
// 	cout << "[DEBUG] function run called" << endl;
// 	pollfd serverPollFd;
// 	serverPollFd.fd = serverSocket;
// 	serverPollFd.events = POLLIN;
// 	clientFds.push_back(serverPollFd);

// 	while (true)
// 	{
// 		int pollResult = poll(&clientFds[0], clientFds.size(), -1);
// 		if (pollResult == -1)
// 		{
// 			cerr << "Error in poll" << endl;
// 			break;
// 		}

// 		for (size_t i = 0; i < clientFds.size(); ++i)
// 		{
// 			if (clientFds[i].revents & POLLIN)
// 			{
// 				if (clientFds[i].fd == serverSocket)
// 				{
// 					handleNewConnection();
// 				} else
// 				{
// 					handleClientData(i);
// 				}
// 			}
// 		}
// 	}
// }

// In Server.cpp, modify the run() function:

void Server::run()
{
    cout << "[DEBUG] function run called" << endl;
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
                cout << "[DEBUG] Client fd " << clientFds[i].fd
                     << " has pending data, adding POLLOUT" << endl;
                clientFds[i].events |= POLLOUT;
            }
        }

        int pollResult = poll(&clientFds[0], clientFds.size(), 1000); // 1 second timeout
        if (pollResult == -1)
        {
            cerr << "Error in poll" << endl;
            break;
        }

        cout << "[DEBUG] Poll returned " << pollResult << " events" << endl;

        for (size_t i = 0; i < clientFds.size(); ++i)
        {
            cout << "[DEBUG] Checking fd " << clientFds[i].fd
                 << " revents: " << clientFds[i].revents << endl;

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
                cout << "[DEBUG] POLLOUT event for client fd " << clientFds[i].fd << endl;
                // Try to send pending data
                clients[clientFds[i].fd].tryFlushSendBuffer();
            }
        }
    }
}

void Server::handleNewConnection()
{
    cout << "[DEBUG] function handleNewConnection called" << endl;
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == -1)
    {
        cerr << "Error accepting client connection: " << strerror(errno) << endl;
        return;
    }

    cout << "[DEBUG] Accepted new connection on socket " << clientSocket << endl;

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

    cout << "[DEBUG] New client connected: " << hostBuffer
         << " (fd: " << clientSocket << ")" << endl;
}

// void Server::handleNewConnection()
// {
// 	cout << "[DEBUG] function handleNewConnection called" << endl;
// 	sockaddr_in clientAddr;
// 	socklen_t clientAddrLen = sizeof(clientAddr);
// 	int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
// 	if (clientSocket == -1)
// 	{
// 		cerr << "Error accepting client connection: " << strerror(errno) << endl;
// 		return;
// 	}

// 	setNonBlocking(clientSocket);

// 	char hostBuffer[INET_ADDRSTRLEN];
// 	if (inet_ntop(AF_INET, &(clientAddr.sin_addr), hostBuffer, INET_ADDRSTRLEN) == NULL)
// 	{
// 		cerr << "Error converting client address to string: " << strerror(errno) << endl;
// 		close(clientSocket);
// 		return;
// 	}

// 	clients[clientSocket] = Client(clientSocket, string(hostBuffer));

// 	pollfd clientPollFd;
// 	clientPollFd.fd = clientSocket;
// 	clientPollFd.events = POLLIN;
// 	clientFds.push_back(clientPollFd);

// 	cout << "New client connected: " << hostBuffer << endl;
// }

void Server::handleClientData(size_t index)
{
    cout << "[DEBUG] function handleClientData called for client fd: " << clientFds[index].fd << endl;
    char buffer[1024];
    int clientFd = clientFds[index].fd;
    int bytesRead = recv(clientFds[index].fd, buffer, sizeof(buffer), 0);


	cout << "\n[RECV] fd " << clientFds[index].fd << " got " << bytesRead << " bytes:" << endl;
	cout << "[RECV] Raw hex: ";
	for (int i = 0; i < bytesRead; i++) {
		printf("%02x ", (unsigned char)buffer[i]);
	}
	cout << endl;
	cout << "[RECV] ASCII: ";
	for (int i = 0; i < bytesRead; i++) {
		if (isprint(buffer[i])) cout << buffer[i];
		else cout << '.';
	}
	cout << endl;
    cout << "[DEBUG] Received bytes: " << bytesRead << endl;
    cout << "[DEBUG] Raw data: [";
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
            cout << "[DEBUG] Processing message: " << completeMessages[i] << endl;

            // Parse the message first
            Message parsedMessage(completeMessages[i]);

            // Regular command processing
            if (parsedMessage.getCommand() == "PASS")
            {
                if (parsedMessage.getParams().empty())
                {
                    send(clientFd, "Password required\r\n", 19, 0);
                    return;
                }
                string password = parsedMessage.getParams()[0];
                if (clients[clientFd].isAuthenticated())
                {
                    cout << "Client already authenticated" << endl;
                    return;
                }
                if (authenticateClient(password, clientFd))
                {
                    send(clientFd, "Password accepted\r\n", 19, 0);
                }
                else
                {
                    send(clientFd, "Invalid password\r\n", 18, 0);
                    close(clientFd);
                    clientFds.erase(clientFds.begin() + index);
                    clients.erase(clientFd);
                }
            }
            else if (!clients[clientFd].isAuthenticated())
            {
                send(clientFd, "Password required\r\n", 19, 0);
            }
            else
            {
                map<string, CommandHandler*>::iterator handler;

                // Use appropriate handler based on message type
                if (!completeMessages[i].empty() && completeMessages[i][0] == ':')
                {
                    // Use ServerMessageHandler for messages starting with :
                    cout << "[DEBUG] Using ServerMessageHandler for message: " << completeMessages[i] << endl;
                    handler = commandHandlers.find("SERVER_MESSAGE");
                }
                else
                {
                    // Use regular command handler
                    handler = commandHandlers.find(parsedMessage.getCommand());
                }

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
	cout << "[DEBUG] function authenticateClient called" << endl;
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
	cout << "[DEBUG] function getServerName called" << endl;
	return "YourIRCServer";
}

string Server::getVersion()
{
	cout << "[DEBUG] function getVersion called" << endl;
	return "1.0";
}

string Server::getCreationDate()
{
	cout << "[DEBUG] function getCreationDate called" << endl;
	return "May 1, 2023";
}

void Server::removeClient(int fd)
{
	cout << "[DEBUG] function removeClient called" << endl;
	// Remove the client from the clients map
	map<int, Client>::iterator it = clients.find(fd);
	if (it != clients.end())
	{
		clients.erase(it);
	}

	// Remove the client's file descriptor from the pollfd vector
	vector<pollfd>::iterator pollIt = find_if(clientFds.begin(), clientFds.end(), FdComparer(fd));
	if (pollIt != clientFds.end())
	{
		clientFds.erase(pollIt);
	}
}

vector<Channel*> Server::getClientChannels(const Client& client)
{
	cout << "[DEBUG] function getClientChannels called" << endl;
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
	cout << "[DEBUG] function broadcastMessage called" << endl;
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
	cout << "[DEBUG] function isNicknameInUse called" << endl;
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
    map<int, Client>::iterator it = clients.begin();
    while (it != clients.end()) {
        map<int, Client>::iterator current = it++;  // Store current and advance iterator
        if (current->second.isPingTimedOut()) {
            std::cout << "[DEBUG] Client " << current->first << " ping timed out, disconnecting" << std::endl;
            removeClient(current->first);
            clients.erase(current);  // Erase using stored iterator
        } else if (current->second.needsPing()) {
            std::cout << "[DEBUG] Sending ping to client " << current->first << std::endl;
            current->second.sendPing();
        }
    }
}
