#ifndef WHOIS_COMMAND_HANDLER_HPP
#define WHOIS_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class WhoisCommandHandler : public CommandHandler {
private:
	void sendNoNicknameError(Client& client) const;
	void sendNoSuchNickError(Client& client, const string& targetNick) const;
	void sendWhoisInfo(Client& client, Client* target) const;
	Client* findAndValidateTarget(Client& client, const string& targetNick) const;

public:
	WhoisCommandHandler(Server& server);
	virtual void handle(Client& client, const Message& message);
};

#endif
