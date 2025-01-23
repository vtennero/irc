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
#include "KickCommandHandler.hpp"
#include "NamesCommandHandler.hpp"
#include "TopicCommandHandler.hpp"
#include "InviteCommandHandler.hpp"
#include "WhoCommandHandler.hpp"
#include "PassCommandHandler.hpp"

#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <signal.h>
#include <stdio.h>

#define CUSTOM_INET_ADDRSTRLEN 16  // Length of "255.255.255.255" + null terminator


// sockets
struct CustomAddr {
	unsigned short family;
	unsigned char data[14];
};

void setupAddress(struct sockaddr* addr, int port) {
    CustomAddr* custom = reinterpret_cast<CustomAddr*>(addr);
    std::memset(custom, 0, sizeof(CustomAddr));  // Zero out the entire structure

    custom->family = AF_INET;

    // Set port in network byte order in first 2 bytes of data
    unsigned short netPort = htons(port);
    std::memcpy(&custom->data[0], &netPort, sizeof(netPort));

    // Set address in network byte order in next 4 bytes
    // INADDR_ANY is already 0 from memset
    // If you need a specific address, you would set it here like:
    // unsigned long netAddr = htonl(specificAddr);
    // std::memcpy(&custom->data[2], &netAddr, sizeof(netAddr));
}

// Helper function for custom IP to string conversion
static void byte_to_str(unsigned char byte, char* buffer) {
	char reverse[4];
	int idx = 0;

	if (byte == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}

	while (byte > 0) {
		reverse[idx++] = '0' + (byte % 10);
		byte /= 10;
	}

	for (int i = 0; i < idx; i++)
		buffer[i] = reverse[idx - 1 - i];
	buffer[idx] = '\0';
}

const char* custom_ip_to_str(int af, const void* src, char* dst, size_t size) {
	if (af != AF_INET || !src || !dst || size < CUSTOM_INET_ADDRSTRLEN) {
		return NULL;
	}

	const unsigned char* addr = static_cast<const unsigned char*>(src);
	char temp[4];
	size_t len = 0;

	for (int i = 0; i < 4; i++) {
		byte_to_str(addr[i], temp);
		size_t part_len = strlen(temp);

		if (i > 0) {
			if (len >= size - 1) return NULL;
			dst[len++] = '.';
		}

		if (len + part_len >= size) return NULL;
		strcpy(dst + len, temp);
		len += part_len;
	}

	return dst;
}

// Nickname database
bool Server::addAuthNick(const string& nickname, const string& password)
{
	if (password.empty())
	{
		return false;
	}

	for (string::const_iterator it = password.begin(); it != password.end(); ++it)
	{
		if (!isalnum(*it))
		{
			return false;
		}
	}
	authednicks[nickname] = password;
	return true;
}

bool Server::isNickAuthed(const string& nickname) const
{
	return authednicks.find(nickname) != authednicks.end();
}

string Server::getAuthPassword(const string& nickname) const
{
	map<string, string>::const_iterator it = authednicks.find(nickname);
	if (it != authednicks.end())
	{
		return it->second;
	}
	return "";
}

// this one is to check if identify was run on time + if clients timeout
void Server::checkClientPings()
{
	vector<int> timeoutFds;

	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.isPingTimedOut())
		{
			timeoutFds.push_back(it->first);
		} else if (it->second.needsPing())
		{
			it->second.sendPing();
		}

		if (it->second.hasAuthTimedOut())
		{
			cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET
				 << " Auth timeout for client " << it->first << endl;
			it->second.setNickname("*");
			it->second.setAwaitAuth(false);
			it->second.send(":" + getServerName() + " NOTICE " + it->second.getNickname() +
						  " :Authentication timeout. Nickname reset.\r\n");
		}
	}

	for (vector<int>::iterator it = timeoutFds.begin(); it != timeoutFds.end(); ++it)
	{
		removeClient(*it);
	}
}
int Server::getSocket() const
{
	return serverSocket;
}

vector<Channel*> Server::getAllChannels()
{
		vector<Channel*> result;
		for (map<string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			result.push_back(&(it->second));
		}
		return result;
	};

// listing commands
void Server::listAvailableCommands()
{
	cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET "Available commands:" << endl;
	for (map<string, CommandHandler*>::iterator it = commandHandlers.begin();
		 it != commandHandlers.end(); ++it)
		 {
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET "  " << it->first << endl;
	}
}

// cleaning and signal handling
Server* Server::instance = NULL;

void Server::setupSignalHandling()
{
	instance = this;
	struct sigaction sa;
	sa.sa_handler = &Server::signalHandler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error setting up signal handler: " << strerror(errno) << endl;
	}
}

void Server::signalHandler(int signum)
{
	if (signum == SIGINT)
	{
		cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Received SIGINT (Ctrl+C). Cleaning up..." << endl;
		if (instance != NULL)
		{  // Changed nullptr to NULL
			instance->cleanup();
		}
		exit(0);
	}
}

void Server::cleanup()
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Performing server cleanup..." << endl;

	// Close all client connections
	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		close(it->first);
	}
	clients.clear();
	clientFds.clear();

	if (serverSocket != -1)
	{
		close(serverSocket);
		serverSocket = -1;
	}

	for (map<string, CommandHandler*>::iterator it = commandHandlers.begin();
		 it != commandHandlers.end(); ++it)
		 {
		delete it->second;
	}
	commandHandlers.clear();

	channels.clear();

	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Cleanup complete. Exiting..." << endl;
}

// privmsg
void Server::sendToClient(const string& nickname, const string& message)
{
	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
		{
			it->second.send(message);
			return;
		}
	}
}

void Server::broadcastToChannel(const string& channelName, const string& message, const Client* exclude)
{
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end())
	{
		it->second.broadcastMessage(message, exclude);
	}
}

Client* Server::findClientByNickname(const string& nickname)
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Looking for: " << nickname << endl;
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Current clients in server:" << endl;

	for (map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		cout << "  FD: " << it->first
				  << " Nick: " << it->second.getNickname()
				  << " Registered: " << (it->second.isRegistered() ? "Yes" : "No")
				  << endl;

		if (it->second.getNickname() == nickname)
		{
			cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Found matching client!" << endl;
			return &(it->second);
		}
	}

	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " No matching client found" << endl;
	return NULL;
}

bool Server::isChannelExist(const string& channelName) const
{
	return channels.find(channelName) != channels.end();
}

Channel* Server::getChannel(const string& channelName)
{
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end())
	{
		return &(it->second);
	}
	return NULL;
}

// Replace createChannel implementation
Channel* Server::createChannel(const string& channelName)
{
	map<string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end())
	{
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
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error setting socket to non-blocking: " << strerror(errno) << endl;
	}
}

Server::Server(int port, const string& password) : serverSocket(-1), serverPassword(password)
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

	// Create socket with allowed parameters
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error creating socket: " << strerror(errno) << endl;
		return;
	}

	// Enable address reuse
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error setting socket options: " << strerror(errno) << endl;
		close(serverSocket);
		serverSocket = -1;
		return;
	}

	struct sockaddr addr;
	setupAddress(&addr, port);

	if (bind(serverSocket, &addr, sizeof(addr)) == -1)
	{
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error binding socket: " << strerror(errno) << endl;
		close(serverSocket);
		serverSocket = -1;
		return;
	}

	if (listen(serverSocket, 5) == -1)
	{
		cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error listening on socket: " << strerror(errno) << endl;
		close(serverSocket);
		serverSocket = -1;
		return;
	}

	setNonBlocking(serverSocket);


	commandHandlers["NICK"] = new NickCommandHandler(*this);
	commandHandlers["USER"] = new UserCommandHandler(*this);
	commandHandlers["QUIT"] = new QuitCommandHandler(*this);
	commandHandlers["PRIVMSG"] = new PrivmsgCommandHandler(*this);
	commandHandlers["MODE"] = new ModeCommandHandler(*this);
	commandHandlers["WHOIS"] = new WhoisCommandHandler(*this);
	commandHandlers["KICK"] = new KickCommandHandler(*this);
	commandHandlers["PING"] = new PingCommandHandler(*this);
	commandHandlers["PONG"] = new PongCommandHandler(*this);
	commandHandlers["JOIN"] = new JoinCommandHandler(*this);
	commandHandlers["PART"] = new PartCommandHandler(*this);
	commandHandlers["LIST"] = new ListCommandHandler(*this);
	commandHandlers["NAMES"] = new NamesCommandHandler(*this);
	commandHandlers["TOPIC"] = new TopicCommandHandler(*this);
	commandHandlers["INVITE"] = new InviteCommandHandler(*this);
	commandHandlers["WHO"] = new WhoCommandHandler(*this);
	commandHandlers["PASS"] = new PassCommandHandler(*this);

	cout << "Server listening on port " << port << endl;

	setupSignalHandling();
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET "Server listening on port " << port << endl;
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


void Server::run() {
    cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

    pollfd serverPollFd;
    serverPollFd.fd = serverSocket;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;
    clientFds.push_back(serverPollFd);

    while (true) {
        checkClientPings();  // Keep ping checks

        // Update poll events for clients with pending data
        for (size_t i = 1; i < clientFds.size(); ++i) {
            Client& client = clients[clientFds[i].fd];
            clientFds[i].events = POLLIN;
            if (client.hasDataToSend()) {
                clientFds[i].events |= POLLOUT;
            }
        }

        int pollResult = poll(&clientFds[0], clientFds.size(), 1000);
        if (pollResult == -1) {
            if (errno == EINTR) continue;  // Handle interrupt
            cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error in poll: " << strerror(errno) << endl;
            break;
        }

        for (size_t i = 0; i < clientFds.size(); ++i) {
            if (clientFds[i].revents & (POLLHUP | POLLERR)) {
                if (clientFds[i].fd != serverSocket) {
                    removeClient(clientFds[i].fd);
                    continue;
                }
            }

            if (clientFds[i].revents & POLLIN) {
                if (clientFds[i].fd == serverSocket) {
                    handleNewConnection();
                } else {
                    handleClientData(i);
                }
            }

            if (clientFds[i].revents & POLLOUT) {
                try {
                    clients[clientFds[i].fd].tryFlushSendBuffer();
                } catch (const std::exception& e) {
                    cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Send error, removing client: " << e.what() << endl;
                    removeClient(clientFds[i].fd);
                }
            }
        }
    }
}

void Server::handleNewConnection()
{
    cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
    struct sockaddr clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, &clientAddr, &clientAddrLen);
    if (clientSocket == -1)
    {
        cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET "Error accepting client connection: " << strerror(errno) << endl;
        return;
    }

    // Set non-blocking mode
    setNonBlocking(clientSocket);

    // Set smaller send buffer
    int sndBufSize = 4096;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, &sndBufSize, sizeof(sndBufSize)) == -1)
    {
        cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET "Warning: Could not set send buffer size: " << strerror(errno) << endl;
    }

    // Extract IP address from generic sockaddr
    char hostBuffer[CUSTOM_INET_ADDRSTRLEN];
    CustomAddr* custom = reinterpret_cast<CustomAddr*>(&clientAddr);

    // In our custom structure, the IP address starts after the port (2 bytes into data array)
    unsigned char* addr_bytes = &custom->data[2];

    // Convert network byte order address to string
    snprintf(hostBuffer, CUSTOM_INET_ADDRSTRLEN, "%d.%d.%d.%d",
             addr_bytes[0], addr_bytes[1], addr_bytes[2], addr_bytes[3]);

    // Create client and add to poll list
    clients[clientSocket] = Client(clientSocket, string(hostBuffer));

    pollfd clientPollFd;
    clientPollFd.fd = clientSocket;
    clientPollFd.events = POLLIN;
    clientPollFd.revents = 0;
    clientFds.push_back(clientPollFd);

    cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " New client connected: " << hostBuffer << endl;
}
void Server::handleClientData(size_t index) {
    cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << clientFds[index].fd << endl;
    char buffer[1024];
    int clientFd = clientFds[index].fd;

    try {
        int bytesRead = recv(clientFds[index].fd, buffer, sizeof(buffer), 0);

        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Client disconnected normally" << endl;
            } else {
                cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error reading from client: " << strerror(errno) << endl;
            }
            removeClient(clientFd);
            return;
        }

        // Process received data
        map<int, Client>::iterator clientIt = clients.find(clientFd);
        if (clientIt == clients.end()) {
            return;
        }

        clientIt->second.appendToBuffer(string(buffer, bytesRead));
        vector<string> completeMessages = clientIt->second.getCompleteMessages();

        for (size_t i = 0; i < completeMessages.size(); ++i) {
            try {
                Message parsedMessage(completeMessages[i]);

                // Only allow PASS, PING and QUIT before authentication
                if (!clientIt->second.isGuestenticated() &&
                    parsedMessage.getCommand() != "PASS" &&
                    parsedMessage.getCommand() != "PING" &&
                    parsedMessage.getCommand() != "QUIT") {
                    clientIt->second.send("464 * :Password required\r\n");
                    continue;
                }

                // Handle commands through appropriate handlers
                map<string, CommandHandler*>::iterator handler = commandHandlers.find(parsedMessage.getCommand());
                if (handler != commandHandlers.end()) {
                    handler->second->handle(clientIt->second, parsedMessage);
                }

            } catch (const std::exception& e) {
                cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error processing message: " << e.what() << endl;
                continue;
            }
        }
    } catch (const std::exception& e) {
        cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error handling client data: " << e.what() << endl;
        removeClient(clientFd);
    }
}

// void Server::handleClientData(size_t index) {
//     cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called for client fd: " << clientFds[index].fd << endl;
//     char buffer[1024];
//     int clientFd = clientFds[index].fd;

//     try {
//         int bytesRead = recv(clientFds[index].fd, buffer, sizeof(buffer), 0);

//         if (bytesRead <= 0) {
//             if (bytesRead == 0) {
//                 cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Client disconnected normally" << endl;
//             } else {
//                 cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error reading from client: " << strerror(errno) << endl;
//             }
//             removeClient(clientFd);
//             return;
//         }

//         // Process received data
//         map<int, Client>::iterator clientIt = clients.find(clientFd);
//         if (clientIt == clients.end()) {
//             return;
//         }

//         clientIt->second.appendToBuffer(string(buffer, bytesRead));
//         vector<string> completeMessages = clientIt->second.getCompleteMessages();

//         for (size_t i = 0; i < completeMessages.size(); ++i) {
//             try {
//                 Message parsedMessage(completeMessages[i]);
//                 if (parsedMessage.getCommand() == "PING") {
//                     // Immediately respond to PING
//                     string pongReply = "PONG :" + parsedMessage.getParams()[0] + "\r\n";
//                     clientIt->second.send(pongReply);
//                     continue;
//                 }

//                 if (parsedMessage.getCommand() == "PASS") {
//                     // Handle password authentication directly
//                     if (guestenticateClient(parsedMessage.getParams()[0], clientFd)) {
//                         clientIt->second.send(":" + getServerName() + " NOTICE * :Password accepted\r\n");
//                     } else {
//                         clientIt->second.send("464 * :Password incorrect\r\n");
//                         // removeClient(clientFd);
//                         return;
//                     }
//                     continue;
//                 }

//                 // Only process other commands if client is authenticated
//                 if (!clientIt->second.isGuestenticated() && parsedMessage.getCommand() != "QUIT") {
//                     clientIt->second.send("464 * :Password required\r\n");
//                     continue;
//                 }

//                 map<string, CommandHandler*>::iterator handler = commandHandlers.find(parsedMessage.getCommand());
//                 if (handler != commandHandlers.end()) {
//                     handler->second->handle(clientIt->second, parsedMessage);
//                 }
//             } catch (const std::exception& e) {
//                 cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error processing message: " << e.what() << endl;
//                 continue;
//             }
//         }
//     } catch (const std::exception& e) {
//         cerr << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Error handling client data: " << e.what() << endl;
//         removeClient(clientFd);
//     }
// }

bool Server::guestenticateClient(const string& password, int clientFd) {
    cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;

    // Validate input
    if (password.empty()) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Empty password" << endl;
        return false;
    }

    // Find client
    map<int, Client>::iterator it = clients.find(clientFd);
    if (it == clients.end()) {
        cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Client not found" << endl;
        return false;
    }

    Client& client = it->second;

    // Prevent re-authentication
    if (client.isGuestenticated()) {
        cout << YELLOW "[" << __PRETTY_FUNCTION__ << "]" RESET " Client already authenticated" << endl;
        return true;
    }

    // Validate password
    if (password == serverPassword) {
        cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Password accepted" << endl;
        client.setGuestenticated(true);
        return true;
    }

    cout << RED "[" << __PRETTY_FUNCTION__ << "]" RESET " Password rejected" << endl;
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

void Server::removeClient(int fd)
{
	cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " called for fd: " << fd << endl;


	// First, remove from channels to avoid dangling pointers
	for (map<string, Channel>::iterator chIt = channels.begin(); chIt != channels.end(); ++chIt)
	{
		chIt->second.removeClient(&clients[fd]);
	}

	// Close the socket first
	close(fd);

	// Remove from clients map
	map<int, Client>::iterator clientIt = clients.find(fd);
	if (clientIt != clients.end())
	{
		clients.erase(clientIt);
	}

	// Remove from pollfd vector
	vector<pollfd>::iterator pollIt = find_if(clientFds.begin(), clientFds.end(), FdComparer(fd));
	if (pollIt != clientFds.end())
	{
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
		// we will need to update because we are removing client.nickname
		{
			return true;
		}
	}
	return false;
}
