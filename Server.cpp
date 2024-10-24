#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include "NickCommandHandler.hpp"
#include "UserCommandHandler.hpp"
#include "QuitCommandHandler.hpp"

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


// Helper function for std::find_if
struct FdComparer {
    FdComparer(int fd) : fd_to_find(fd) {}
    bool operator()(const pollfd& pfd) const {
        return pfd.fd == fd_to_find;
    }
    int fd_to_find;
};

void Server::setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error setting socket to non-blocking: " << strerror(errno) << std::endl;
    }
}

Server::Server(int port, const std::string& password) : serverSocket(-1), serverPassword(password) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        close(serverSocket);
        return;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
        close(serverSocket);
        return;
    }

    setNonBlocking(serverSocket);

    commandHandlers["NICK"] = new NickCommandHandler(*this);
    commandHandlers["USER"] = new UserCommandHandler(*this);
    commandHandlers["QUIT"] = new QuitCommandHandler(*this);

    std::cout << "Server listening on port " << port << std::endl;
}

Server::~Server() {
    if (serverSocket != -1) {
        close(serverSocket);
    }
    for (size_t i = 0; i < clientFds.size(); ++i) {
        close(clientFds[i].fd);
    }

    std::map<std::string, CommandHandler*>::iterator it;
    for (it = commandHandlers.begin(); it != commandHandlers.end(); ++it) {
        delete it->second;
    }
}

void Server::run() {
    pollfd serverPollFd;
    serverPollFd.fd = serverSocket;
    serverPollFd.events = POLLIN;
    clientFds.push_back(serverPollFd);

    while (true) {
        int pollResult = poll(&clientFds[0], clientFds.size(), -1);
        if (pollResult == -1) {
            std::cerr << "Error in poll" << std::endl;
            break;
        }

        for (size_t i = 0; i < clientFds.size(); ++i) {
            if (clientFds[i].revents & POLLIN) {
                if (clientFds[i].fd == serverSocket) {
                    handleNewConnection();
                } else {
                    handleClientData(i);
                }
            }
        }
    }
}

void Server::handleNewConnection() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        std::cerr << "Error accepting client connection: " << strerror(errno) << std::endl;
        return;
    }

    setNonBlocking(clientSocket);

    char hostBuffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(clientAddr.sin_addr), hostBuffer, INET_ADDRSTRLEN) == NULL) {
        std::cerr << "Error converting client address to string: " << strerror(errno) << std::endl;
        close(clientSocket);
        return;
    }

    clients[clientSocket] = Client(clientSocket, std::string(hostBuffer));

    pollfd clientPollFd;
    clientPollFd.fd = clientSocket;
    clientPollFd.events = POLLIN;
    clientFds.push_back(clientPollFd);

    std::cout << "New client connected: " << hostBuffer << std::endl;
}

void Server::handleClientData(size_t index) {
    char buffer[1024];
    int clientFd = clientFds[index].fd;
    int bytesRead = recv(clientFds[index].fd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            std::cout << "Client disconnected" << std::endl;
        } else {
            std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
        }
        close(clientFds[index].fd);
        clientFds.erase(clientFds.begin() + index);
 } else {
        std::string message(buffer, bytesRead);
        Message parsedMessage(message);
        
        if (parsedMessage.getCommand() == "PASS") {
            std::string password = parsedMessage.getParams()[0];
            if (authenticateClient(password, clientFd)) {
                send(clientFd, "Password accepted\r\n", 19, 0);
            } else {
                send(clientFd, "Invalid password\r\n", 18, 0);
                close(clientFd);
                clientFds.erase(clientFds.begin() + index);
                clients.erase(clientFd);
            }
        } else if (!clients[clientFd].isAuthenticated()) {
            send(clientFd, "Password required\r\n", 19, 0);
        } else {
            std::map<std::string, CommandHandler*>::iterator handler = commandHandlers.find(parsedMessage.getCommand());
            if (handler != commandHandlers.end()) {
                handler->second->handle(clients[clientFd], parsedMessage);
            } else {
                // Unknown command
                std::cout << "Unknown command: " << parsedMessage.getCommand() << std::endl;
            }
        }
    }
}

bool Server::authenticateClient(const std::string& password, int clientFd) {
    if (password == serverPassword) {
        clients[clientFd].setAuthenticated(true);
        return true;
    }
    return false;
}

std::string Server::getServerName() {
    return "YourIRCServer";
}

std::string Server::getVersion() {
    return "1.0";
}

std::string Server::getCreationDate() {
    return "May 1, 2023";
}

void Server::removeClient(int fd) {
    // Remove the client from the clients map
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it != clients.end()) {
        clients.erase(it);
    }

    // Remove the client's file descriptor from the pollfd vector
    std::vector<pollfd>::iterator pollIt = std::find_if(clientFds.begin(), clientFds.end(), FdComparer(fd));
    if (pollIt != clientFds.end()) {
        clientFds.erase(pollIt);
    }
}

std::vector<Channel*> Server::getClientChannels(const Client& client) {
    std::vector<Channel*> clientChannels;
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it->second.hasClient(&client)) {
            clientChannels.push_back(&(it->second));
        }
    }
    return clientChannels;
}

void Server::broadcastMessage(const std::string& message, const Client* exclude) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (&(it->second) != exclude) {
            it->second.send(message);
        }
    }
}