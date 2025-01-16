#ifndef USER_COMMAND_HANDLER_HPP
#define USER_COMMAND_HANDLER_HPP

#include "CommandHandler.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class UserCommandHandler : public CommandHandler {
public:
	UserCommandHandler(Server& server);
	virtual void handle(Client& client, const Message& message);
private:
	void sendWelcomeMessages(Client& client);
};

#endif // USERCOMMANDHANDLER_HPP
