#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <sys/poll.h>
#include <cstddef>
#include <string>
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include <map>

class Server {
private:
    int serverSocket;
    std::vector<pollfd> clientFds;
    static const int MAX_CLIENTS = 100;
    std::string serverPassword;
    std::vector<bool> clientAuthenticated;
    std::map<std::string, CommandHandler*> commandHandlers;
    std::map<int, Client> clients;
    std::map<std::string, Channel> channels;


    void setNonBlocking(int socket);
    void handleNewConnection();
    void handleClientData(size_t index);
    bool authenticateClient(const std::string& password, int clientFd);

public:
    Server(int port, const std::string& password);
    ~Server();
    void run();

    static std::string getServerName();
    static std::string getVersion();
    static std::string getCreationDate();

    void removeClient(int fd);
    std::vector<Channel*> getClientChannels(const Client& client);
    void broadcastMessage(const std::string& message, const Client* exclude = NULL);

};

#endif // SERVER_HPP