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

class Client
{
	private:
		int fd;
		string hostname;
		string nickname;
		string username;
		string realname;
		bool guestenticated;
		bool registered;
		string messageBuffer;
		string sendBuffer;
		time_t lastPingSent;
		time_t lastPongReceived;
		string lastPingToken;
		bool awaitingPong;
		void trimLeadingWhitespace();
		string extractNextMessage(size_t& pos);
		bool isWhitespace(char c) const;
		void debugPrintServerMessage(const string& message) const;
	public:
		Client(int fd);
		Client();
		Client(int fd, const string& hostname);

		void setNickname(const string& nick);
		void setUsername(const string& user);
		void setRealname(const string& real);
		void setGuestenticated(bool guest);
		void setRegistered(bool reg);
		string getNickname() const;
		string getUsername() const;
		bool isGuestenticated() const;
		bool isRegistered() const;
		bool hasDataToSend() const;
		void updateLastPongReceived();

		int getFd() const;
		void send(const string& message);

		void setHostname(const string& hostname);
		string getHostname() const;

		// buffer mgt
		void appendToBuffer(const string& data);
		vector<string> getCompleteMessages();
		void tryFlushSendBuffer();

		bool isPingTimedOut() const;
		void sendPing();
		bool verifyPongToken(const string& token);
		bool needsPing() const;
};

#endif
