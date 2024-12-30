#include "Client.hpp"
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <stdio.h>
#include <ostream>      // And add this:
#include <string>
#include <ctime>
#include <unistd.h>


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

void hexDump(const char* data, size_t size) {
    std::cout << "[DEBUG] Hex dump of message (" << size << " bytes):" << std::endl;
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", (unsigned char)data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

void Client::tryFlushSendBuffer() {
    while (!sendBuffer.empty()) {
        ssize_t sent = ::send(fd, sendBuffer.c_str(), sendBuffer.length(), MSG_DONTWAIT);

        if (sent > 0) {
            cout << "[DEBUG] Sent " << sent << "/" << sendBuffer.length()
                 << " bytes to fd " << fd << endl;
            sendBuffer.erase(0, sent);
        }
        else if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                cout << "[DEBUG] Socket buffer full for fd " << fd
                     << ", " << sendBuffer.length() << " bytes pending" << endl;
                return;
            }
            else {
                cerr << "Error sending to fd " << fd << ": "
                     << strerror(errno) << endl;
                sendBuffer.clear();
                return;
            }
        }
    }
}

void Client::send(const std::string& message) {
    // Check if message already ends with \r\n
    std::string completeMessage = message;
    if (completeMessage.length() < 2 ||
        completeMessage.substr(completeMessage.length() - 2) != "\r\n") {
        completeMessage += "\r\n";
    }

    cout << "[DEBUG] Queueing message to fd " << fd << " ("
         << completeMessage.length() << " bytes)" << endl;

    hexDump(completeMessage.c_str(), completeMessage.length());

    // Add to send buffer
    sendBuffer += completeMessage;

    // Add small delay
    usleep(100000); // 100ms delay

    // Try to send immediately
    tryFlushSendBuffer();
}

void Client::sendPing() {
    char buffer[32];
    sprintf(buffer, "ping_%ld", static_cast<long>(time(NULL)));
    lastPingToken = buffer;
    lastPingSent = time(NULL);
    awaitingPong = true;

    std::string pingMsg = "PING :" + lastPingToken + "\r\n";
    send(pingMsg);
}

bool Client::isPingTimedOut() const {
    if (!awaitingPong) return false;
    return (time(NULL) - lastPingSent) > 60; // 60 second timeout
}

bool Client::needsPing() const {
    return !awaitingPong && (time(NULL) - lastPongReceived) > 120; // Send ping every 120 seconds
}

bool Client::verifyPongToken(const std::string& token) {
    if (token == lastPingToken) {
        awaitingPong = false;
        lastPongReceived = time(NULL);
        return true;
    }
    return false;
}

int Client::getFd() const
{
	std::cout << "[DEBUG] function getFd called" << std::endl;
	return fd;
}

void Client::appendToBuffer(const std::string& data)
{
    std::cout << "[DEBUG] function appendToBuffer called with data length: " << data.length() << std::endl;
    std::cout << "[DEBUG] Raw data being appended (hex): ";
    for (size_t i = 0; i < data.length(); i++) {
        printf("%02x ", (unsigned char)data[i]);
    }
    std::cout << std::endl;
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
        std::cout << "[DEBUG] Trimming leading character: " << (int)messageBuffer[0] << std::endl;
        messageBuffer.erase(0, 1);
    }

    while ((pos = messageBuffer.find("\r\n")) != std::string::npos)
    {
        std::string completeMessage = messageBuffer.substr(0, pos);
        std::cout << "[DEBUG] Found message ending at pos " << pos << ": " << completeMessage << std::endl;

        // Special debug for server messages
        if (!completeMessage.empty() && completeMessage[0] == ':') {
            std::cout << "[DEBUG] Found server message: " << completeMessage << std::endl;
        }

        completeMessages.push_back(completeMessage);
        messageBuffer.erase(0, pos + 2);

        // Trim again after extracting a message
        while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' '))
        {
            std::cout << "[DEBUG] Trimming trailing character: " << (int)messageBuffer[0] << std::endl;
            messageBuffer.erase(0, 1);
        }
    }

    return completeMessages;
}
