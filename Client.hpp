#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <iostream>
#include <ctime>
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class Client {
private:
	int fd;
	string hostname;
	string nickname;
	string username;
	string realname;
	bool authenticated;
	bool registered;
	string messageBuffer;
	string sendBuffer;
	time_t lastPingSent;
	time_t lastPongReceived;
	string lastPingToken;
	bool awaitingPong;

public:
	Client(int fd);
	Client();
	Client(int fd, const string& hostname);

	int getFd() const;
	void setNickname(const string& nick) { nickname = nick; }
	void setUsername(const string& user) { username = user; }
	void setRealname(const string& real) { realname = real; }
	void setAuthenticated(bool auth) { authenticated = auth; }
	void setRegistered(bool reg) { registered = reg; }
	string getNickname() const { return nickname; }
	string getUsername() const { return username; }
	bool isAuthenticated() const { return authenticated; }
	bool isRegistered() const { return registered; }
	void send(const string& message);

	void setHostname(const string& hostname);
	string getHostname() const;

	// buffer mgt
	void appendToBuffer(const string& data);
	vector<string> getCompleteMessages();
	bool hasDataToSend() const { return !sendBuffer.empty(); }
	void tryFlushSendBuffer();

	void updateLastPongReceived() { lastPongReceived = time(NULL); }
	bool isPingTimedOut() const;
	void sendPing();
	bool verifyPongToken(const string& token);
	bool needsPing() const;
};

#endif
