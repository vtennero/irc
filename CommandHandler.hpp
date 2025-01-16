#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include "Client.hpp"
#include "Message.hpp"
# include "Debug.hpp"

class Server;

class CommandHandler {
protected:
	Server& server;

public:
	CommandHandler(Server& server);
	virtual ~CommandHandler();
	virtual void handle(Client& client, const Message& message) = 0;
};

#endif // COMMANDHANDLER_HPP
