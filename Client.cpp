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
	cout << "[DEBUG] function Client default constructor called" << endl;
}

Client::Client(int fd) : fd(fd), nickname("*"), authenticated(false), registered(false)
{
	cout << "[DEBUG] function Client constructor (fd) called with fd: " << fd << endl;
}

Client::Client(int fd, const string& hostname)
    : fd(fd),
      hostname(hostname),
      nickname("*"),
      username(""),
      authenticated(false),
      registered(false),
      messageBuffer(""),
      sendBuffer(""),
      lastPingSent(time(NULL)),
      lastPongReceived(time(NULL)),
      lastPingToken(""),
      awaitingPong(false)
{
    cout << "[DEBUG] function Client constructor (fd, hostname) called with fd: " << fd
         << ", hostname: " << hostname << endl;
}

void Client::setHostname(const string& hostname)
{
	cout << "[DEBUG] function setHostname called with hostname: " << hostname << endl;
	this->hostname = hostname;
}

string Client::getHostname() const
{
	cout << "[DEBUG] function getHostname called" << endl;
	return hostname;
}

void hexDump(const char* data, size_t size) {
    cout << "[DEBUG] Hex dump of message (" << size << " bytes):" << endl;
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

void Client::send(const string& message) {
    // Check if message already ends with \r\n
    string completeMessage = message;
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

    string pingMsg = "PING :" + lastPingToken + "\r\n";
    send(pingMsg);
}

// bool Client::isPingTimedOut() const {
//     if (!awaitingPong) return false;
//     return (time(NULL) - lastPingSent) > 60; // 60 second timeout
// }

bool Client::isPingTimedOut() const {
    if (!awaitingPong) {
        return false;
    }
    time_t currentTime = time(NULL);
    cout << "[DEBUG] Checking ping timeout - Last ping: " << lastPingSent
         << ", Current: " << currentTime
         << ", Diff: " << (currentTime - lastPingSent) << endl;
    return (currentTime - lastPingSent) > 60; // 60 second timeout
}

bool Client::needsPing() const {
    return !awaitingPong && (time(NULL) - lastPongReceived) > 120; // Send ping every 120 seconds
}

bool Client::verifyPongToken(const string& token) {
    if (token == lastPingToken) {
        awaitingPong = false;
        lastPongReceived = time(NULL);
        return true;
    }
    return false;
}

int Client::getFd() const
{
	cout << "[DEBUG] function getFd called" << endl;
	return fd;
}

void Client::appendToBuffer(const string& data)
{
    cout << "[DEBUG] function appendToBuffer called with data length: " << data.length() << endl;
    cout << "[DEBUG] Raw data being appended (hex): ";
    for (size_t i = 0; i < data.length(); i++) {
        printf("%02x ", (unsigned char)data[i]);
    }
    cout << endl;
    messageBuffer += data;
    cout << "DEBUG - Current buffer: [" << messageBuffer << "]" << endl;
}

vector<string> Client::getCompleteMessages()
{
    cout << "[DEBUG] function getCompleteMessages called" << endl;
    vector<string> completeMessages;
    size_t pos;

    // Trim any leading whitespace/newlines
    while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' '))
    {
        cout << "[DEBUG] Trimming leading character: " << (int)messageBuffer[0] << endl;
        messageBuffer.erase(0, 1);
    }

    while ((pos = messageBuffer.find("\r\n")) != string::npos)
    {
        string completeMessage = messageBuffer.substr(0, pos);
        cout << "[DEBUG] Found message ending at pos " << pos << ": " << completeMessage << endl;

        // Special debug for server messages
        if (!completeMessage.empty() && completeMessage[0] == ':') {
            cout << "[DEBUG] Found server message: " << completeMessage << endl;
        }

        completeMessages.push_back(completeMessage);
        messageBuffer.erase(0, pos + 2);

        // Trim again after extracting a message
        while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' '))
        {
            cout << "[DEBUG] Trimming trailing character: " << (int)messageBuffer[0] << endl;
            messageBuffer.erase(0, 1);
        }
    }

    return completeMessages;
}
