#ifndef QUITCOMMANDHANDLER_HPP
#define QUITCOMMANDHANDLER_HPP

#include "CommandHandler.hpp"
#include "Server.hpp"

class QuitCommandHandler : public CommandHandler {
public:
	QuitCommandHandler(Server& server) : CommandHandler(server) {}
	virtual void handle(Client& client, const Message& message);
};

#endif // QUITCOMMANDHANDLER_HPP
