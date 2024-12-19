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

void Client::appendToBuffer(const std::string& data) {
	messageBuffer += data;
	std::cout << "DEBUG - Current buffer: [" << messageBuffer << "]" << std::endl;
}

std::vector<std::string> Client::getCompleteMessages() {
	std::vector<std::string> completeMessages;
	size_t pos;

	// Trim any leading whitespace/newlines
	while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' ')) {
		messageBuffer.erase(0, 1);
	}

	while ((pos = messageBuffer.find("\r\n")) != std::string::npos) {
		std::string completeMessage = messageBuffer.substr(0, pos);
		completeMessages.push_back(completeMessage);
		messageBuffer.erase(0, pos + 2);

		// Trim again after extracting a message
		while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' ')) {
			messageBuffer.erase(0, 1);
		}
	}

	return completeMessages;
}
