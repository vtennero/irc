#ifndef NICK_COMMAND_HANDLER_HPP
#define NICK_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"
#include <map>
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class NickCommandHandler : public CommandHandler {
public:
	NickCommandHandler(Server& server);
	bool isValidNickname(const string& nick);
	virtual void handle(Client& client, const Message& message);
};

#endif // NICKCOMMANDHANDLER_HPP
