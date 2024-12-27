#include "Client.hpp"
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <iostream>

Client::Client() : fd(-1), nickname("*"), authenticated(false), registered(false)
{
	std::cout << "[DEBUG] function Client default constructor called" << std::endl;
}

Client::Client(int fd) : fd(fd), nickname("*"), authenticated(false), registered(false)
{
	std::cout << "[DEBUG] function Client constructor (fd) called with fd: " << fd << std::endl;
}

Client::Client(int fd, const std::string& hostname)
	: fd(fd),
	  hostname(hostname),
	  nickname("*"),
	  username(""),
	  authenticated(false),
	  registered(false)
{
	std::cout << "[DEBUG] function Client constructor (fd, hostname) called with fd: " << fd << ", hostname: " << hostname << std::endl;
}

void Client::setHostname(const std::string& hostname)
{
	std::cout << "[DEBUG] function setHostname called with hostname: " << hostname << std::endl;
	this->hostname = hostname;
}

std::string Client::getHostname() const
{
	std::cout << "[DEBUG] function getHostname called" << std::endl;
	return hostname;
}

void Client::send(const std::string& message) {
    std::string completeMessage = message + "\r\n";
    std::cout << "[DEBUG] Sending to client fd " << fd << ": " << message;
    ssize_t total = 0;
    ssize_t remaining = completeMessage.length();

    while (total < (ssize_t)completeMessage.length()) {
        ssize_t sent = ::send(fd, completeMessage.c_str() + total, remaining, 0);
        if (sent == -1) {
            std::cerr << "Error sending message: " << strerror(errno) << std::endl;
            return;
        }
        total += sent;
        remaining -= sent;
    }
    std::cout << "[DEBUG] Successfully sent " << total << " bytes" << std::endl;
}

int Client::getFd() const
{
	std::cout << "[DEBUG] function getFd called" << std::endl;
	return fd;
}

void Client::appendToBuffer(const std::string& data)
{
	std::cout << "[DEBUG] function appendToBuffer called with data length: " << data.length() << std::endl;
	messageBuffer += data;
	std::cout << "DEBUG - Current buffer: [" << messageBuffer << "]" << std::endl;
}

std::vector<std::string> Client::getCompleteMessages()
{
	std::cout << "[DEBUG] function getCompleteMessages called" << std::endl;
	std::vector<std::string> completeMessages;
	size_t pos;

	// Trim any leading whitespace/newlines
	while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' '))
	{
		messageBuffer.erase(0, 1);
	}

	while ((pos = messageBuffer.find("\r\n")) != std::string::npos)
	{
		std::string completeMessage = messageBuffer.substr(0, pos);
		completeMessages.push_back(completeMessage);
		messageBuffer.erase(0, pos + 2);

		// Trim again after extracting a message
		while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' '))
		{
			messageBuffer.erase(0, 1);
		}
	}

	return completeMessages;
}
