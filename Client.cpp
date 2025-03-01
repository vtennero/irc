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
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
}

Client::Client(int fd) : fd(fd), nickname("*"), authenticated(false), registered(false)
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with fd: " << fd << endl;
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
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with fd: " << fd << ", hostname: " << hostname << endl;
}

void Client::setHostname(const string& hostname)
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with hostname: " << hostname << endl;
	this->hostname = hostname;
}

string Client::getHostname() const
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
	return hostname;
}

void hexDump(const char* data, size_t size) {
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Hex dump of message (" << size << " bytes):" << endl;
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
			cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Sent " << sent << "/" << sendBuffer.length() << " bytes to fd " << fd << endl;
			sendBuffer.erase(0, sent);
		}
		else if (sent == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Socket buffer full for fd " << fd << ", " << sendBuffer.length() << " bytes pending" << endl;
				return;
			}
			else {
				cerr << "Error sending to fd " << fd << ": " << strerror(errno) << endl;
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

	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Queueing message to fd " << fd << " (" << completeMessage.length() << " bytes)" << endl;

	// uncomment to view hex dump of message
	// hexDump(completeMessage.c_str(), completeMessage.length());

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


bool Client::isPingTimedOut() const {
	if (!awaitingPong) {
		return false;
	}
	time_t currentTime = time(NULL);
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Checking ping timeout - Last ping: " << lastPingSent  << ", Current: " << currentTime << ", Diff: " << (currentTime - lastPingSent) << endl;
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
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
	return fd;
}

void Client::appendToBuffer(const string& data)
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with data length: " << data.length() << endl;
	// cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Raw data being appended (hex): ";
	for (size_t i = 0; i < data.length(); i++) {
		printf("%02x ", (unsigned char)data[i]);
	}
	cout << endl;
	messageBuffer += data;
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Current buffer: [" << messageBuffer << "]" << endl;
}

vector<string> Client::getCompleteMessages()
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
	vector<string> completeMessages;
	size_t pos;

	// Trim any leading whitespace/newlines
	while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' '))
	{
		cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Trimming leading character: " << (int)messageBuffer[0] << endl;
		messageBuffer.erase(0, 1);
	}

	while ((pos = messageBuffer.find("\r\n")) != string::npos)
	{
		string completeMessage = messageBuffer.substr(0, pos);
		cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Found message ending at pos " << pos << ": " << completeMessage << endl;

		// Special debug for server messages
		if (!completeMessage.empty() && completeMessage[0] == ':') {
			cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Found server message: " << completeMessage << endl;
		}

		completeMessages.push_back(completeMessage);
		messageBuffer.erase(0, pos + 2);

		// Trim again after extracting a message
		while (!messageBuffer.empty() && (messageBuffer[0] == '\n' || messageBuffer[0] == '\r' || messageBuffer[0] == ' '))
		{
			cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Trimming trailing character: " << (int)messageBuffer[0] << endl;
			messageBuffer.erase(0, 1);
		}
	}

	return completeMessages;
}
