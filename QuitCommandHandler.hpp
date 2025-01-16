#ifndef QUITCOMMANDHANDLER_HPP
#define QUITCOMMANDHANDLER_HPP

#include "CommandHandler.hpp"
#include "Server.hpp"
# include "Debug.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;
using std::ostringstream;

class QuitCommandHandler : public CommandHandler {
public:
	QuitCommandHandler(Server& server);
	virtual void handle(Client& client, const Message& message);
};

#endif // QUITCOMMANDHANDLER_HPP
