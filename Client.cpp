#include "Client.hpp"
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <iostream>

Client::Client() : fd(-1), authenticated(false), registered(false) {}

Client::Client(int fd) : fd(fd), authenticated(false), registered(false) {}

Client::Client(int fd, const std::string& hostname)
    : fd(fd), hostname(hostname), authenticated(false), registered(false) {}

void Client::setHostname(const std::string& hostname) {
    this->hostname = hostname;
}

std::string Client::getHostname() const {
    return hostname;
}

void Client::send(const std::string& message) {
    if (::send(fd, message.c_str(), message.length(), 0) == -1) {
        std::cerr << "Error sending message: " << strerror(errno) << std::endl;
    }
}

int Client::getFd() const { return fd; }
