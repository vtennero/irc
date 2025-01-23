#include "Client.hpp"
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <stdio.h>
#include <ostream>
#include <string>
#include <ctime>
#include <unistd.h>

void Client::setNickname(const string& nick)
{
	string oldNick = nickname;
	nickname = nick;

	if (oldNick != "*" && oldNick != nick)
	{
		send(":" + oldNick + "!" + username + "@" + hostname + " NICK :" + nick + "\r\n");
	}
}

void Client::setUsername(const string& user)
{
	username = user;
}

void Client::setRealname(const string& real)
{
	realname = real;
}

void Client::setGuestenticated(bool guest)
{
	guestenticated = guest;
}

void Client::setRegistered(bool reg)
{
	registered = reg;
}

string Client::getNickname() const
{
	return nickname;
}

string Client::getUsername() const
{
	return username;
}

bool Client::isGuestenticated() const
{
	return guestenticated;
}

bool Client::isRegistered() const
{
	return registered;
}

bool Client::isAuthenticated() const
{
	return authenticated;
}

void Client::setAuthenticated()
{
	authenticated = true;
	awaitAuth = false;
	authTimeout = 0;
	send(":" + nickname + " NOTICE " + nickname + " :You are now authenticated\r\n");
}

bool Client::hasDataToSend() const
{
	return !sendBuffer.empty();
}

void Client::updateLastPongReceived()
{
	lastPongReceived = time(NULL);
}

void Client::setAwaitAuth(bool await, time_t timeout)
{
	awaitAuth = await;
	authTimeout = timeout;
}

bool Client::isAwaitingAuth() const
{
	return awaitAuth;
}

time_t Client::getAuthTimeout() const
{
	return authTimeout;
}

bool Client::hasAuthTimedOut() const
{
	if (!awaitAuth) return false;
	return time(NULL) > authTimeout;
}



Client::Client() : fd(-1), nickname("*"), guestenticated(false), registered(false), authenticated(false)
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called" << endl;
}

Client::Client(int fd) : fd(fd), nickname("*"), guestenticated(false), registered(false), authenticated(false)
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " called with fd: " << fd << endl;
}

Client::Client(int fd, const string& hostname)
	: fd(fd),
	  hostname(hostname),
	  nickname("*"),
	  username(""),
	  guestenticated(false),
	  registered(false),
	  authenticated(false),
	  messageBuffer(""),
	  sendBuffer(""),
	  lastPingSent(time(NULL)),
	  lastPongReceived(time(NULL)),
	  lastPingToken(""),
	  awaitingPong(false),
		awaitAuth(false),
	  authTimeout(0)
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


void hexDump(const char* data, size_t size)
{
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Hex dump of message (" << size << " bytes):" << endl;
	for (size_t i = 0; i < size; i++)
	{
		printf("%02x ", (unsigned char)data[i]);
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n");
}

void Client::tryFlushSendBuffer() {
    while (!sendBuffer.empty()) {
        ssize_t sent = ::send(fd, sendBuffer.c_str(), sendBuffer.length(), MSG_DONTWAIT);

        if (sent > 0) {
            cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Sent " << sent << "/" << sendBuffer.length() << " bytes to fd " << fd << endl;
            sendBuffer.erase(0, sent);
        } else if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                cout << GREEN "[" << __PRETTY_FUNCTION__ << "]" RESET " Socket buffer full for fd " << fd << ", " << sendBuffer.length() << " bytes pending" << endl;
                return;
            } else {
                // Connection error - throw exception to be caught by caller
                throw std::runtime_error(std::string("Send error: ") + strerror(errno));
                sendBuffer.clear(); // Clear buffer on error
            }
        }
    }
}

void Client::send(const string& message)
{
	// Check if message already ends with \r\n
	string completeMessage = message;
	if (completeMessage.length() < 2 ||
		completeMessage.substr(completeMessage.length() - 2) != "\r\n")
		{
		completeMessage += "\r\n";
	}

	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Queueing message to fd " << fd << " (" << completeMessage.length() << " bytes)" << endl;

	// uncomment to view hex dump of message
	// hexDump(completeMessage.c_str(), completeMessage.length());

	// Add to send buffer
	sendBuffer += completeMessage;
	tryFlushSendBuffer();
}

void Client::sendPing()
{
	char buffer[32];
	sprintf(buffer, "ping_%ld", static_cast<long>(time(NULL)));
	lastPingToken = buffer;
	lastPingSent = time(NULL);
	awaitingPong = true;

	string pingMsg = "PING :" + lastPingToken + "\r\n";
	send(pingMsg);
}


bool Client::isPingTimedOut() const
{
	if (!awaitingPong)
	{
		return false;
	}
	time_t currentTime = time(NULL);
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Checking ping timeout - Last ping: " << lastPingSent  << ", Current: " << currentTime << ", Diff: " << (currentTime - lastPingSent) << endl;
	return (currentTime - lastPingSent) > 60; // 60 second timeout
}

bool Client::needsPing() const
{
	return !awaitingPong && (time(NULL) - lastPongReceived) > 120; // Send ping every 120 seconds
}

bool Client::verifyPongToken(const string& token)
{
	if (token == lastPingToken)
	{
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
	for (size_t i = 0; i < data.length(); i++)
	{
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

	trimLeadingWhitespace();

	while ((pos = messageBuffer.find("\r\n")) != string::npos)
	{
		string completeMessage = extractNextMessage(pos);
		completeMessages.push_back(completeMessage);
		trimLeadingWhitespace();
	}

	return completeMessages;
}

void Client::trimLeadingWhitespace()
{
	while (!messageBuffer.empty() && isWhitespace(messageBuffer[0]))
	{
		cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Trimming leading character: "
			 << (int)messageBuffer[0] << endl;
		messageBuffer.erase(0, 1);
	}
}

string Client::extractNextMessage(size_t& pos)
{
	string completeMessage = messageBuffer.substr(0, pos);
	cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Found message ending at pos "
		 << pos << ": " << completeMessage << endl;

	debugPrintServerMessage(completeMessage);

	messageBuffer.erase(0, pos + 2);  // +2 to skip \r\n
	return completeMessage;
}

bool Client::isWhitespace(char c) const
{
	return (c == '\n' || c == '\r' || c == ' ');
}

void Client::debugPrintServerMessage(const string& message) const
{
	if (!message.empty() && message[0] == ':')
	{
		cout << BRIGHT_BLUE "[" << __PRETTY_FUNCTION__ << "]" RESET " Found server message: "
			 << message << endl;
	}
}
