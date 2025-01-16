#ifndef NICK_COMMAND_HANDLER_HPP
#define NICK_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include <map>
#include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class NickCommandHandler : public CommandHandler
{
private:
	bool validateNicknameRequest(Client& client, const Message& message, string& newNick);
	bool validateNicknameFormat(const string& nick);
	bool validateFirstCharacter(char c);
	bool validateCharacter(char c);
	void handleNicknameChange(Client& client, const string& newNick);
	void sendNickChangeNotification(Client& client, const string& oldNick, const string& newNick);
	void checkAndHandleRegistration(Client& client);

public:
	NickCommandHandler(Server& server);
	bool isValidNickname(const string& nick);
	virtual void handle(Client& client, const Message& message);
};

#endif
